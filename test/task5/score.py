import re
import sys
import argparse
import subprocess as subps
import os.path as osp
import gc

sys.path.append(osp.abspath(__file__ + "/../.."))
from common import CasesHelper, ScoreReport, print_parsed_args

TIME_OUT: int = 20
COMP_PATH: str = None
QEMU_PATH: str = None

class Error(Exception):
    pass


def get_tm(sp):
    ret = -1
    # 匹配复数形式 (a+bi 或 a-bi)，或者普通整数（正负数）
    matchObj = re.findall(r"Return Code: ([-+]?\d*)", sp)
    if matchObj:  # 检查是否有匹配结果
        result = matchObj[-1]
        # 处理普通整数
        ret = int(result)
    return ret


def score_one(
    cases_helper: CasesHelper, case: CasesHelper.Case
) -> ScoreReport.TestEntry:
    """评测单个测例，打印出一行评测结果"""

    name = case.name
    score = 0.0
    max_score = 100.0
    output = "[PASS]"
    weight = case.weight
    output_path, fp = cases_helper.open_case_report(case)
    input_path, input_fp = cases_helper.open_case_input(case)
    print(output_path, end=" ... ", flush=True)

    def fprint(*args):
        print(*args, file=fp)

    with fp:
        try:
            # 如果没有标准答案的相关文件，零分
            answer_out_path = cases_helper.of_case_bindir("answer.out", case)
            answer_err_path = cases_helper.of_case_bindir("answer.err", case)
            if not osp.exists(answer_out_path):
                output = "没有可参考的标准答案"
                fprint("标准参考答案文件不存在：", answer_out_path)
                raise Error()
            if not osp.exists(answer_err_path):
                output = "没有可参考的标准答案"
                fprint("标准参考答案文件不存在：", answer_err_path)
                raise Error()

            # 如果没有 output.s 文件，零分
            judge_answer_path = cases_helper.of_case_bindir("output.s", case)
            if not osp.exists(judge_answer_path):
                output = "没有输出结果"
                fprint("输出结果文件不存在：", judge_answer_path)
                raise Error()

            # 尝试编译学生答案
            output_exe_path = cases_helper.of_case_bindir("output.exe", case)
            try:
                with open(
                    cases_helper.of_case_bindir("output.compile", case),
                    "w",
                    encoding="utf-8",
                ) as f:
                    subps.run(
                        [
                            COMP_PATH,
                            "--static",
                            "-o",
                            output_exe_path,
                            judge_answer_path,
                            RTLIB_PATH,
                        ],
                        stdout=f,
                        stderr=f,
                        timeout=TIME_OUT,
                    )
            except Exception as e:
                output = "编译输出结果时出错"
                fprint("编译输出结果时出错：", e)
                fprint("出错行数：", e.__traceback__.tb_lineno)
                raise Error(e)

            # 尝试运行用户答案
            try:
                output_out_path = cases_helper.of_case_bindir("output.out", case)
                output_err_path = cases_helper.of_case_bindir("output.err", case)
                with open(output_out_path, "w", encoding="utf-8") as f, open(
                    output_err_path, "w", encoding="utf-8"
                ) as ferr:
                    retn = subps.run(
                        [QEMU_PATH, output_exe_path],
                        stdin=input_fp,
                        stdout=f,
                        stderr=ferr,
                        timeout=TIME_OUT,
                    ).returncode
                    ferr.write(f"Return Code: {retn}\n")
            except subps.TimeoutExpired as e:
                output = "运行输出结果超时"
                fprint("运行输出结果超时：", e)
                raise Error(e)
            except Exception as e:
                output = "运行输出结果时出错"
                fprint("运行输出结果时出错：", e)
                fprint("出错行数：", e.__traceback__.tb_lineno)
                raise Error(e)

            # 比较输出
            with open(answer_out_path, "r", encoding="utf-8") as f:
                answer_out = f.read()
            with open(output_out_path, "r", encoding="utf-8") as f:
                output_out = f.read()
            with open(answer_err_path, "r", encoding="utf-8") as f:
                answer_err = f.read()
            with open(output_err_path, "r", encoding="utf-8") as f:
                output_err = f.read()

            answer_ret = get_tm(answer_err)
            output_ret = get_tm(output_err)
            
            if answer_out != output_out:
                fprint("\n输出不匹配")
                fprint(">----")
                fprint("标准答案: %s" % answer_out)
                fprint("用户答案: %s" % output_out)
                fprint("<----")
                output = "输出不匹配"
                raise Error()

            if answer_ret != output_ret:
                fprint("\n返回值不匹配")
                fprint(">----")
                fprint("标准答案: %d" % answer_ret)
                fprint("用户答案: %d" % output_ret)
                fprint("<----")
                output = "返回值不匹配"
                raise Error()

            score = max_score
            fprint("")
            fprint(f"得分：{score:.2f}/{max_score:.2f}")

        except Error:
            pass

    print(output)
    return ScoreReport.TestEntry(
        name=name,
        score=score,
        max_score=max_score,
        output=output,
        output_path=output_path,
        weight=weight,
    )


def score_all(cases_helper: CasesHelper) -> ScoreReport:
    """评测所有测例，生成成绩单"""

    score_report = ScoreReport("task5")

    for case in cases_helper.cases:
        try:
            test_entry = score_one(cases_helper, case)
        except Exception:
            continue
        score_report.tests.append(test_entry)
        gc.collect()

    score_report.leader_board.append(
        ScoreReport.LeaderBoardEntry(
            "总分",
            score_report.final_score(),
            0,
            True,
            "",
        )
    )

    return score_report

def generate_score_report(
    cases_helper: CasesHelper,
    title: str,
    root_report_name: str,
    autograder_json_name: str,
) -> None:
    """评分，生成成绩单，打印成绩单，保存成绩单

    :param CaseHelper cases_helper: 需要评分的测例
    :param str title: 评分报告标题
    :param str root_report_name: 总评分报告文件名
    :param str autograder_json_name: Autograder 使用的 JSON 文件的名字
    """
    
    print()
    score_report = score_all(cases_helper)
    print()
    print("=" * 80)
    score_report.print()
    print("=" * 80)

    # 保存成绩单
    txt_path, f = cases_helper.open_root_report(root_report_name)
    with f:
        score_report.dump_human_text(f)
    print("成绩单已保存：", cases_helper.of_bindir(root_report_name))

    json_path, f = cases_helper.open_autograder_json(autograder_json_name)
    with f:
        score_report.dump_autograder(f)
    print("JSON 格式：", cases_helper.of_bindir(autograder_json_name))

if __name__ == "__main__":
    parser = argparse.ArgumentParser("实验五评测脚本")
    parser.add_argument("srcdir", type=str, help="测例目录")
    parser.add_argument("bindir", type=str, help="测评输出目录")
    parser.add_argument("cases_file", type=str, help="测例表路径")
    parser.add_argument("ctest_exe", type=str, help="CTest 程序路径")
    parser.add_argument("comp_path", type=str, help="编译器路径")
    parser.add_argument("qenu_path", type=str, help="qemu路径")
    parser.add_argument("rtlib_path", type=str, help="运行时库路径")
    parser.add_argument("--single", type=str, help="运行单个测例")
    args = parser.parse_args()
    print_parsed_args(parser, args)

    print("加载测例表...", end="", flush=True)
    cases_helper = CasesHelper.load_file(
        args.srcdir,
        args.bindir,
        args.cases_file,
    )
    print("完成")

    COMP_PATH = args.comp_path
    QEMU_PATH = args.qenu_path
    RTLIB_PATH = args.rtlib_path

    if case_name := args.single:
        for case in cases_helper.cases:
            if case.name == case_name:
                break
        else:
            print("没有找到指定的测例：", case_name)
            sys.exit(1)

        if case_name.startswith("llm"):
            # 通过 CTest 运行同学们的代码
            case_outerr = cases_helper.open_case_outerr(case, "ctest")
            out_path, out, err_path, err = case_outerr
            print("CTest 运行日志：", out_path, err_path)
            print("运行 CTest 以得到结果...", end="", flush=True)
            with out, err:
                subps.run(
                    [
                        args.ctest_exe,
                        "--test-dir",
                        args.bindir,
                        "-R",
                        "^task5-llm/" + case_name,
                    ],
                    stdout=out,
                    stderr=err,
                )
        else:
            # 通过 CTest 运行同学们的代码
            case_outerr = cases_helper.open_case_outerr(case, "ctest")
            out_path, out, err_path, err = case_outerr
            print("CTest 运行日志：", out_path, err_path)
            print("运行 CTest 以得到结果...", end="", flush=True)
            with out, err:
                subps.run(
                    [
                        args.ctest_exe,
                        "--test-dir",
                        args.bindir,
                        "-R",
                        "^task5-classic/" + case_name,
                    ],
                    stdout=out,
                    stderr=err,
                )
        print("完成")

        score_one(cases_helper, case)
        print("评测结果已保存：", cases_helper.of_case_bindir("score.txt", case))

    else:
        # 通过 CTest 运行同学们的代码
        out_path, out, err_path, err = cases_helper.open_outerr("ctest")
        print("CTest 运行日志：", out_path, err_path)
        print("运行 CTest 以得到结果...", end="", flush=True)
        with out, err:
            subps.run(
                [args.ctest_exe, "--test-dir", args.bindir, "-R", "^task5/.*"],
                stdout=out,
                stderr=err,
            )
        print("完成")

        classic_cases = []
        llm_cases = []
        for cases in cases_helper.cases:
            if cases.name.startswith("llm"):
                llm_cases.append(cases)
            else:
                classic_cases.append(cases)
        classic_cases_helper = CasesHelper(
            srcdir=cases_helper.srcdir, bindir=cases_helper.bindir, cases=classic_cases
        )
        llm_cases_helper = CasesHelper(
            srcdir=cases_helper.srcdir, bindir=cases_helper.bindir, cases=llm_cases
        )

        generate_score_report(
            classic_cases_helper, "task5-classic", "score-classic.txt", "score-classic.json"
        )
        generate_score_report(
            llm_cases_helper, "task5-llm", "score-llm.txt", "score-llm.json"
        )

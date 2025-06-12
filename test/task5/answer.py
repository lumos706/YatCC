"""对给定的测例表调用 `YatCC` 获取汇编代码，
将输出保存到同名输出目录下的 answer.s文件中。
然后再调用 `arm-linux-gnueabihf-gcc` 编译 answer.s 为 answer.exe，
再运行 answer.exe，将输出保存到 answer.out 和 answer.err 文件中。
"""

import sys
import os.path as osp
import argparse
import subprocess as subps
import tempfile
import re
import os

sys.path.append(osp.abspath(__file__ + "/../.."))
from common import (
    CasesHelper,
    print_parsed_args,
    exit_if_cases_cached,
    cache_cases,
)

if __name__ == "__main__":
    parser = argparse.ArgumentParser("实验五答案生成", description=__doc__)
    parser.add_argument("srcdir", help="测例源码目录")
    parser.add_argument("bindir", help="输出目录")
    parser.add_argument("cases_file", help="测例表路径")
    parser.add_argument("gcc", help="gcc程序路径")
    parser.add_argument("YatCC", help="YatCC程序路径")
    parser.add_argument("qemu_path", help="qemu程序路径")
    parser.add_argument("rtlib", help="运行时库源码路径")
    parser.add_argument("rtlib_a", help="运行时库文件路径")
    args = parser.parse_args()
    print_parsed_args(parser, args)

    cache = exit_if_cases_cached(args.bindir, args.cases_file)

    print("加载测例表...", end="", flush=True)
    cases_helper = CasesHelper.load_file(
        args.srcdir,
        args.bindir,
        args.cases_file,
    )
    print("完成")

    for case in cases_helper.cases:
        # 生成汇编代码
        asm_path = cases_helper.of_case_bindir("answer.s", case, True)
        print(asm_path, end=" ... ", flush=True)
        try:
            # 读取源文件并删除 include 语句
            src_path = osp.join(args.srcdir, case.name)
            with open(src_path, "r", encoding="utf-8") as src_file:
                src_content = src_file.read()
            
            ll_path = cases_helper.of_case_bindir("answer.ll", case, True)
            with open(ll_path, "w", encoding="utf-8") as ll_file:
                retn = subps.run(
                    [
                        args.YatCC,
                        src_path,
                        asm_path,
                    ],
                    stdout=ll_file,
                    timeout=30,
                ).returncode
        except subps.TimeoutExpired:
            print("TIMEOUT")
            continue
        if retn:
            print("FAIL", retn)
            exit(1)
        print("OK")

        # 再将汇编代码编译为二进制程序
        exe_path = cases_helper.of_case_bindir("answer.exe", case, True)
        print(exe_path, end=" ... ", flush=True)
        try:
            with open(
                cases_helper.of_case_bindir("answer.compile", case), "w", encoding="utf-8"
            ) as f:
                retn = subps.run(
                    [
                        args.gcc,
                        "--static",
                        "-o",
                        exe_path,
                        asm_path,
                        args.rtlib_a,
                    ],
                    stdout=f,
                    stderr=f,
                    timeout=30,
                ).returncode
        except subps.TimeoutExpired:
            print("TIMEOUT")
            continue
        if retn:
            print("FAIL", retn)
            exit(2)
        print("OK")

        # 运行二进制程序，得到程序输出
        out_path = cases_helper.of_case_bindir("answer.out", case, True)
        err_path = cases_helper.of_case_bindir("answer.err", case, True)
        print(out_path, end=" ... ", flush=True)
        print("OK")
        print(err_path, end=" ... ", flush=True)
        with open(out_path, "w", encoding="utf-8") as f, open(
            err_path, "w", encoding="utf-8"
        ) as ferr:
            try:
                retn = subps.run(
                    [args.qemu_path, exe_path],
                    stdout=f,
                    stderr=ferr,
                    stdin=cases_helper.open_case_input(case)[1],
                    timeout=20,
                ).returncode
                ferr.write(f"Return Code: {retn}\n")
            except subps.TimeoutExpired:
                print("TIMEOUT")
            else:
                print("OK")

    cache_cases(args.bindir, cache)


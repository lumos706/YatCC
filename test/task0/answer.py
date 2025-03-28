"""对给定的测例表调用 `clang -E` 获取预处理结果，将输出保存到构建目录下的同名文件中。
"""

import sys
import os.path as osp
import argparse
import subprocess as subps

sys.path.append(osp.abspath(__file__ + "/../.."))
from common import (
    CasesHelper,
    print_parsed_args,
    exit_if_cases_cached,
    cache_cases,
)

if __name__ == "__main__":
    parser = argparse.ArgumentParser("实验零答案生成", description=__doc__)
    parser.add_argument("srcdir", help="测例源码目录")
    parser.add_argument("bindir", help="输出目录")
    parser.add_argument("cases_file", help="测例表路径")
    parser.add_argument("clang_exe", help="Clang 程序路径")
    parser.add_argument("rtlib", help="运行时库路径")
    parser.add_argument("--no-cache", action="store_true", help="禁用缓存机制")
    args = parser.parse_args()
    print_parsed_args(parser, args)

    if args.no_cache:
        print("缓存机制已禁用，所有测试用例将被重新计算。")
        cache = None  # 直接跳过缓存
    else:
        cache = exit_if_cases_cached(args.bindir, args.cases_file)

    print("加载测例表...", end="", flush=True)
    cases_helper = CasesHelper.load_file(
        args.srcdir,
        args.bindir,
        args.cases_file,
    )
    print("完成")

    for case in cases_helper.cases:
        path = cases_helper.of_bindir(case.name, True)
        print(path, end=" ... ", flush=True)
        with open(path, "wb") as f:
            subps.run(
                [
                    args.clang_exe,
                    "-E",
                    "-isystem",
                    f"{args.rtlib}/include",
                    osp.join(args.srcdir, case.name),
                ],
                stdout=f,
            )
        print("OK")

    if not args.no_cache:
        cache_cases(args.bindir, cache)

#!/usr/bin/env python3

import argparse
import os
import subprocess


def main():
    parser = argparse.ArgumentParser(description="Utility for running code formatters")
    subparsers = parser.add_subparsers(dest="subcommand")

    clang_parser = subparsers.add_parser("run-clang-format", help="Run clang-format")
    clang_parser.add_argument(
        "--clang-format-executable",
        default="clang-format",
        help="Path to clang-format executable",
    )
    clang_parser.add_argument(
        "--git-executable", default="git", help="Path to git executable"
    )
    clang_parser.add_argument(
        "--preset", choices=["format-check", "format-fix"], default="format-fix"
    )

    cmake_parser = subparsers.add_parser("run-cmake-format", help="Run cmake-format")
    cmake_parser.add_argument(
        "--cmake-format-executable",
        default="cmake-format",
        help="Path to cmake-format executable",
    )
    cmake_parser.add_argument(
        "--git-executable", default="git", help="Path to git executable"
    )
    cmake_parser.add_argument(
        "--preset", choices=["format-check", "format-fix"], default="format-fix"
    )

    args, format_args = parser.parse_known_args()

    if args.subcommand == "run-clang-format":
        # Get the list of C and C++ files checked into git
        cmd = [args.git_executable, "ls-files", "*.[ch]", "*.[ch]pp"]
        output = subprocess.check_output(cmd, universal_newlines=True)
        cmd = [args.clang_format_executable]
        if args.preset == "format-fix":
            cmd += ["-i"]
        else:
            cmd += ["--dry-run", "-Werror"]
        source_files = output.strip().split("\n")
        for source_file in source_files:
            cmd += [source_file]
        subprocess.run(cmd, check=True)

    elif args.subcommand == "run-cmake-format":
        # Get the list of CMake files checked into git
        cmd = [args.git_executable, "ls-files", "*CMakeLists.txt", "*.cmake"]
        print(cmd)
        output = subprocess.check_output(cmd, universal_newlines=True)
        cmd = [args.cmake_format_executable]
        if args.preset == "format-fix":
            cmd += ["-i"]
        else:
            cmd += ["--check"]
        source_files = output.strip().split("\n")
        for source_file in source_files:
            cmd += [source_file]
        subprocess.run(cmd, check=True)


if __name__ == "__main__":
    main()

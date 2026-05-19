#!/usr/bin/env python3
"""
Cross-platform driver for the ambix plugin test suite.

Builds the VST3 plugins and the ambix_testhost CLI host, installs the
Python test dependencies, and runs pytest. Works on macOS (Ninja) and
Windows (Visual Studio + MSBuild). Linux should also work with Ninja
but isn't covered by CI.

Usage:
    python scripts/run_tests.py                # build + test
    python scripts/run_tests.py --no-build     # skip cmake, just test
    python scripts/run_tests.py --no-test      # build only
    python scripts/run_tests.py -- -k mirror   # forward args to pytest
"""

import argparse
import os
import platform
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BUILD_DIR = ROOT / "_build"
TESTS_DIR = ROOT / "tests"
REQ_FILE = TESTS_DIR / "requirements.txt"

IS_WINDOWS = platform.system() == "Windows"


def run(cmd, cwd=None):
    print(f"+ {' '.join(str(c) for c in cmd)}", flush=True)
    subprocess.run([str(c) for c in cmd], cwd=cwd, check=True)


def cmake_configure():
    BUILD_DIR.mkdir(exist_ok=True)
    args = [
        "cmake",
        "-S", ROOT,
        "-B", BUILD_DIR,
        "-DBUILD_VST=OFF",
        "-DBUILD_VST3=ON",
        "-DBUILD_AU=OFF",
        "-DBUILD_STANDALONE=OFF",
        "-DBUILD_VISUALIZER=OFF",
        "-DBUILD_TESTHOST=ON",
        "-DNUM_OUTPUTS_DECODER=64",
        "-DMAX_AMBI_ORDER=7",
    ]
    if IS_WINDOWS:
        args += [
            "-A", "x64",
            f"-DFFTW3_INCLUDE_DIR={ROOT / 'win-libs'}",
            f"-DFFTW3F_LIBRARY={ROOT / 'win-libs' / 'x64' / 'libfftw3f-3.lib'}",
        ]
    else:
        args += ["-G", "Ninja"]
    run(args)


def cmake_build():
    args = ["cmake", "--build", BUILD_DIR, "--parallel"]
    if IS_WINDOWS:
        args += ["--config", "Release"]
    run(args)


def install_python_deps():
    run([sys.executable, "-m", "pip", "install", "--upgrade", "pip"])
    run([sys.executable, "-m", "pip", "install", "-r", REQ_FILE])


def run_pytest(extra_args):
    args = [sys.executable, "-m", "pytest", TESTS_DIR, "-v"]
    args += extra_args
    run(args)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--no-build", action="store_true",
                        help="Skip cmake configure + build")
    parser.add_argument("--no-test", action="store_true",
                        help="Skip pytest (build only)")
    parser.add_argument("--no-deps", action="store_true",
                        help="Skip pip install of test requirements")
    parser.add_argument("pytest_args", nargs=argparse.REMAINDER,
                        help="Extra args forwarded to pytest (prefix with --)")
    opts = parser.parse_args()

    if not opts.no_build:
        cmake_configure()
        cmake_build()

    if not opts.no_test:
        if not opts.no_deps:
            install_python_deps()
        # argparse REMAINDER keeps the leading '--' if the user passed one;
        # strip it so it doesn't reach pytest as a positional arg.
        extra = opts.pytest_args
        if extra and extra[0] == "--":
            extra = extra[1:]
        run_pytest(extra)


if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as e:
        sys.exit(e.returncode)

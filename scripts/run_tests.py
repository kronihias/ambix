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
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BUILD_DIR = ROOT / "_build"
TESTS_DIR = ROOT / "tests"
REQ_FILE = TESTS_DIR / "requirements.txt"

IS_WINDOWS = platform.system() == "Windows"

# Runtime dependencies the test build needs next to the binaries.
# The release installer drops libfftw3f-3.dll into System32 for users; the
# test build has no installer step, so we stage the DLL ourselves into each
# VST3 bundle and next to ambix_testhost.exe. Only ambix_binaural actually
# links to it, but a missing dependency for *one* plugin is enough to make
# pedalboard's scanner choke for all of them in the same session, so the
# safe move is to make the DLL findable for every host process.
WIN_RUNTIME_DLLS = [ROOT / "win-libs" / "x64" / "libfftw3f-3.dll"]


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
    if IS_WINDOWS:
        stage_windows_runtime_dlls()


def stage_windows_runtime_dlls():
    """Place runtime DLLs next to the binaries that need them.

    JUCE produces VST3 bundles laid out as <name>.vst3/Contents/x86_64-win/<name>.vst3
    on Windows; Windows loads dependent DLLs from the directory of the
    DLL being loaded, so we drop runtime libs there. The testhost is a
    plain .exe so the same DLLs go next to it too.
    """
    vst3_dir = BUILD_DIR / "vst3"
    testhost_dir = BUILD_DIR / "testhost"

    targets = [testhost_dir]
    if vst3_dir.is_dir():
        for bundle in vst3_dir.glob("*.vst3"):
            bin_dir = bundle / "Contents" / "x86_64-win"
            if bin_dir.is_dir():
                targets.append(bin_dir)

    for dll in WIN_RUNTIME_DLLS:
        if not dll.is_file():
            print(f"! missing runtime DLL: {dll}", flush=True)
            continue
        for dst_dir in targets:
            dst_dir.mkdir(parents=True, exist_ok=True)
            dst = dst_dir / dll.name
            shutil.copy2(dll, dst)
            print(f"  staged {dll.name} -> {dst}", flush=True)


def diagnose_build():
    """List built artefacts so CI logs show what's actually on disk."""
    print("\n--- _build/vst3 ---", flush=True)
    vst3_dir = BUILD_DIR / "vst3"
    if vst3_dir.is_dir():
        for p in sorted(vst3_dir.rglob("*")):
            if p.is_file():
                print(f"  {p.relative_to(BUILD_DIR)}", flush=True)
    else:
        print("  (missing)", flush=True)

    print("\n--- _build/testhost ---", flush=True)
    th_dir = BUILD_DIR / "testhost"
    if th_dir.is_dir():
        for p in sorted(th_dir.iterdir()):
            print(f"  {p.relative_to(BUILD_DIR)}", flush=True)
    else:
        print("  (missing)", flush=True)
    print("", flush=True)


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

    diagnose_build()

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

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
import json
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


def ensure_vcpkg_baseline(vcpkg_root):
    # The preinstalled vcpkg on GH Actions runners often predates the
    # builtin-baseline pinned in vcpkg.json, and `vcpkg install` then aborts
    # with "failed to git show versions/baseline.json". Fetch the missing
    # commit before invoking cmake.
    manifest = ROOT / "vcpkg.json"
    try:
        baseline = json.loads(manifest.read_text())["builtin-baseline"]
    except (FileNotFoundError, KeyError, json.JSONDecodeError):
        return
    if not (vcpkg_root / ".git").exists():
        return
    have = subprocess.run(
        ["git", "-C", str(vcpkg_root), "cat-file", "-e", f"{baseline}^{{commit}}"],
        capture_output=True,
    )
    if have.returncode == 0:
        return
    print(f"\n[vcpkg] baseline {baseline[:12]} missing in {vcpkg_root}, fetching", flush=True)
    fetch = subprocess.run(
        ["git", "-C", str(vcpkg_root), "fetch", "--depth", "1", "origin", baseline],
    )
    if fetch.returncode != 0:
        # Fetch-by-sha can be refused (uploadpack.allowReachableSHA1InWant);
        # fall back to a full fetch from origin.
        run(["git", "-C", str(vcpkg_root), "fetch", "origin"])


def ensure_vcpkg_root():
    # Resolve a usable vcpkg checkout. Honors VCPKG_ROOT, then GH-Actions'
    # VCPKG_INSTALLATION_ROOT; otherwise clones + bootstraps into
    # ~/vcpkg. FFTW3 (manifest-declared in vcpkg.json) is required on
    # Windows since the prebuilt fallback under win-libs/ was removed.
    for env_var in ("VCPKG_ROOT", "VCPKG_INSTALLATION_ROOT"):
        v = os.environ.get(env_var)
        if v and (Path(v) / "scripts" / "buildsystems" / "vcpkg.cmake").is_file():
            return Path(v)

    home_vcpkg = Path.home() / "vcpkg"
    if not (home_vcpkg / "scripts" / "buildsystems" / "vcpkg.cmake").is_file():
        print(f"\n[vcpkg] bootstrapping into {home_vcpkg}", flush=True)
        if not home_vcpkg.exists():
            run(["git", "clone", "https://github.com/microsoft/vcpkg", str(home_vcpkg)])
        bootstrap = home_vcpkg / ("bootstrap-vcpkg.bat" if IS_WINDOWS else "bootstrap-vcpkg.sh")
        run([str(bootstrap), "-disableMetrics"])
    return home_vcpkg


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
        # FFTW3 comes from vcpkg manifest mode (see vcpkg.json — static lib
        # with SSE2/AVX/AVX2/threads). GH Actions runners expose
        # VCPKG_INSTALLATION_ROOT; local dev boxes set VCPKG_ROOT; otherwise
        # we bootstrap into ~/vcpkg.
        vcpkg_root = ensure_vcpkg_root()
        ensure_vcpkg_baseline(vcpkg_root)
        # In-tree overlay triplet so vcpkg only builds the Release variant of
        # FFTW3 (we don't link a debug fftw3f.lib). Halves the first-time
        # vcpkg build cost.
        overlay = (ROOT / "vcpkg-triplets").as_posix()
        args += [
            "-A", "x64",
            f"-DCMAKE_TOOLCHAIN_FILE={vcpkg_root.as_posix()}/scripts/buildsystems/vcpkg.cmake",
            f"-DVCPKG_OVERLAY_TRIPLETS={overlay}",
            "-DVCPKG_TARGET_TRIPLET=x64-windows-static-md-release",
        ]
    else:
        args += ["-G", "Ninja"]
    run(args)


def cmake_build():
    args = ["cmake", "--build", BUILD_DIR, "--parallel"]
    if IS_WINDOWS:
        args += ["--config", "Release"]
    run(args)


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

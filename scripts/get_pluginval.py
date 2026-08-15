#!/usr/bin/env python3
"""
Fetch the pluginval validator (https://github.com/Tracktion/pluginval).

pluginval is a JUCE-based plugin validator: it scans a plugin, then hammers
it with the checks a real host performs (bus layouts, sample-rate/block-size
changes, parameter sweeps, state save/restore, editor open/close, ...) and
exits non-zero if anything misbehaves. tests/test_pluginval.py runs it over
every VST3 in _build/vst3.

The binary is downloaded from the pinned GitHub release into
_build/tools/pluginval-<version>/ and reused on later runs. Set $PLUGINVAL to
an existing binary to use that instead of downloading anything.

Usage:
    python scripts/get_pluginval.py               # download if needed, print path
    python scripts/get_pluginval.py --print-path  # print path only, never download
"""

import os
import platform
import shutil
import stat
import sys
import time
import urllib.error
import urllib.request
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# Pinned so CI results are reproducible; bump deliberately. Override with
# $PLUGINVAL_VERSION (must be a tag from the releases page).
DEFAULT_VERSION = "v1.0.4"

RELEASE_URL = "https://github.com/Tracktion/pluginval/releases/download/{version}/{asset}"

# asset name, path of the executable inside the extracted archive
_ASSETS = {
    "Darwin":  ("pluginval_macOS.zip",   "pluginval.app/Contents/MacOS/pluginval"),
    "Windows": ("pluginval_Windows.zip", "pluginval.exe"),
    "Linux":   ("pluginval_Linux.zip",   "pluginval"),
}


def version():
    return os.environ.get("PLUGINVAL_VERSION") or DEFAULT_VERSION


def _asset():
    system = platform.system()
    if system not in _ASSETS:
        raise RuntimeError(f"No pluginval build for platform {system!r}")
    return _ASSETS[system]


def install_dir():
    return ROOT / "_build" / "tools" / f"pluginval-{version()}"


def binary_path():
    """Where the pluginval executable lives once installed (may not exist)."""
    _, exe_rel = _asset()
    return install_dir() / exe_rel


def find_pluginval():
    """Return an existing pluginval executable, or None.

    $PLUGINVAL wins so a locally built / system-installed validator can be
    used without touching the network.
    """
    override = os.environ.get("PLUGINVAL")
    if override:
        p = Path(override)
        # Accept the macOS .app bundle as well as the bare executable.
        if p.is_dir() and p.suffix == ".app":
            p = p / "Contents" / "MacOS" / "pluginval"
        if p.is_file():
            return p
        raise RuntimeError(f"$PLUGINVAL points at a non-existent binary: {override}")

    p = binary_path()
    return p if p.is_file() else None


def ensure_pluginval():
    """Return the pluginval executable, downloading the release if needed."""
    found = find_pluginval()
    if found:
        return found

    asset, exe_rel = _asset()
    url = RELEASE_URL.format(version=version(), asset=asset)
    dest = install_dir()
    dest.mkdir(parents=True, exist_ok=True)
    archive = dest / asset

    print(f"[pluginval] downloading {url}", flush=True)
    try:
        for attempt in range(1, 4):  # absorb transient github.com flakes
            try:
                urllib.request.urlretrieve(url, archive)
                break
            except urllib.error.HTTPError as e:
                if 400 <= e.code < 500:  # bad tag / renamed asset — retrying won't help
                    raise RuntimeError(f"{url}: HTTP {e.code} {e.reason}") from None
                raise
            except Exception as e:
                if attempt == 3:
                    raise
                print(f"[pluginval] download failed ({e}), retry {attempt}/3",
                      flush=True)
                time.sleep(5)
        with zipfile.ZipFile(archive) as zf:
            zf.extractall(dest)
    except BaseException:
        # Don't leave a half-populated install dir behind: find_pluginval()
        # keys off the extracted binary, and a stale dir only confuses.
        shutil.rmtree(dest, ignore_errors=True)
        raise
    archive.unlink()

    exe = dest / exe_rel
    if not exe.is_file():
        raise RuntimeError(f"{asset} did not contain {exe_rel}")
    # zipfile drops the executable bit.
    exe.chmod(exe.stat().st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)
    print(f"[pluginval] installed {exe}", flush=True)
    return exe


def main():
    if "--print-path" in sys.argv[1:]:
        found = find_pluginval()
        if not found:
            print("pluginval not installed; run scripts/get_pluginval.py", file=sys.stderr)
            return 1
        print(found)
        return 0
    print(ensure_pluginval())
    return 0


if __name__ == "__main__":
    sys.exit(main())

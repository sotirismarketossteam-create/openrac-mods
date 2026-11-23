#!/usr/bin/env python3
"""Fetch the OpenRAC mod SDK pinned by sdk.lock into .sdk/."""
from __future__ import annotations
import argparse, hashlib, json, shutil, sys, tempfile, urllib.request, zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--from", dest="source", type=Path)
    args = parser.parse_args()
    lock_path = ROOT / "sdk.lock"
    lock = json.loads(lock_path.read_text())
    version = lock["version"]
    expected = (lock.get("sha256") or "").lower()
    if not expected:
        if args.source:
            print("sdk.lock has no SHA-256 pin", file=sys.stderr)
            return 1
        checksum = urllib.request.urlopen(lock["url"] + ".sha256", timeout=60).read().decode().split()
        if not checksum or len(checksum[0]) != 64:
            print("SDK release did not publish a SHA-256 checksum", file=sys.stderr)
            return 1
        expected = checksum[0].lower()
        lock["sha256"] = expected
        lock_path.write_text(json.dumps(lock, indent=2) + "\n")
        print("Pinned SDK SHA-256 in sdk.lock; commit the updated lock.", file=sys.stderr)
    folder = f"openrac-mod-sdk-{version}"
    sdk_root = ROOT / ".sdk"
    stamp = sdk_root / f"{folder}.sha256"
    if (sdk_root / folder / "cmake" / "OpenRACModConfig.cmake").is_file() and stamp.is_file() and stamp.read_text().strip() == expected:
        print(sdk_root / folder)
        return 0
    data = args.source.read_bytes() if args.source else urllib.request.urlopen(lock["url"], timeout=60).read()
    actual = hashlib.sha256(data).hexdigest()
    if actual != expected:
        print(f"SDK checksum mismatch: expected {expected}, got {actual}", file=sys.stderr)
        return 1
    sdk_root.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(dir=sdk_root) as tmp:
        archive = Path(tmp) / "sdk.zip"
        archive.write_bytes(data)
        with zipfile.ZipFile(archive) as zf:
            for name in zf.namelist():
                if name.startswith("/") or ".." in Path(name).parts or not name.startswith(folder + "/"):
                    print(f"Unsafe SDK archive path: {name}", file=sys.stderr)
                    return 1
            zf.extractall(tmp)
        shutil.rmtree(sdk_root / folder, ignore_errors=True)
        (Path(tmp) / folder).rename(sdk_root / folder)
    stamp.write_text(expected + "\n")
    print(sdk_root / folder)
    return 0

if __name__ == "__main__":
    raise SystemExit(main())

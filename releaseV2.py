#!/usr/bin/env python3

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

LIBRARY_FILE = "library.json"
SEMVER_REGEX = r"^\d+\.\d+\.\d+$"


def run(cmd, execute):
    print(f"$ {cmd}")
    if execute:
        subprocess.run(cmd, shell=True, check=True)


def git_output(cmd):
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    if result.returncode != 0:
        return None
    return result.stdout.strip()


def get_last_tag():
    tag = git_output("git describe --tags --abbrev=0")
    if not tag:
        return "0.0.0"
    return tag.lstrip("v")


def validate(version):
    if not re.match(SEMVER_REGEX, version):
        raise ValueError("Formato versione non valido (usa MAJOR.MINOR.PATCH)")


def bump(version, level):
    major, minor, patch = map(int, version.split("."))

    if level == "patch":
        patch += 1
    elif level == "minor":
        minor += 1
        patch = 0
    elif level == "major":
        major += 1
        minor = 0
        patch = 0

    return f"{major}.{minor}.{patch}"

def update_library(version, execute):
    path = Path(LIBRARY_FILE)

    if not path.exists():
        print("⚠ library.json non trovato, skip.")
        return False

    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)

    old = data.get("version", "N/A")

    print(f"library.json: {old} --> {version}")

    if execute:
        data["version"] = version
        with open(path, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2)
            f.write("\n")

    return True


def update_library_(version):
    path = Path(LIBRARY_FILE)
    if not path.exists():
        print("⚠ library.json non trovato, skip.")
        return False

    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)

    old = data.get("version", "N/A")
    data["version"] = version

    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2)
        f.write("\n")

    print(f"✔ library.json: {old} → {version}")
    return True


def main():
    parser = argparse.ArgumentParser(description="Simple release tool")
    parser.add_argument("version", nargs="?", help="Versione manuale (es 1.2.3)")
    parser.add_argument("--patch", action="store_true")
    parser.add_argument("--minor", action="store_true")
    parser.add_argument("--major", action="store_true")
    parser.add_argument("--commit", action="store_true")
    parser.add_argument("--tag", action="store_true")
    parser.add_argument("--push", action="store_true")
    parser.add_argument("--go", action="store_true")

    args = parser.parse_args()

    # Determina versione
    if args.version:
        version = args.version.lstrip("v")
    elif args.patch or args.minor or args.major:
        last = get_last_tag()
        level = "patch" if args.patch else "minor" if args.minor else "major"
        version = bump(last, level)
        print(f"Ultimo tag: {last} → Nuova versione: {version}")
    else:
        print("Specifica una versione o usa --patch/--minor/--major")
        sys.exit(1)

    validate(version)
    tag_version = f"v{version}"

    print("\n=== RELEASE PLAN ===")

    if args.tag:
        updated = update_library(version, args.go)

        if updated and args.commit:
            run("git add library.json", args.go)

    if args.commit:
        run(f'git commit -m "Release {version}"', args.go)

    if args.tag:
        run(f'git tag -a {tag_version} -m "Release {version}"', args.go)

    if args.push:
        run("git push", args.go)
        if args.tag:
            run("git push --tags", args.go)

    if not args.go:
        print("\n⚠ Dry run (aggiungi --go per eseguire)")
    else:
        print("\n🚀 Release completata!")


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"\n❌ Errore: {e}")
        sys.exit(1)
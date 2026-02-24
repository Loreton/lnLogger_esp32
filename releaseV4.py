#!/usr/bin/env python3
import argparse, json, re, subprocess, sys
from pathlib import Path
from datetime import datetime

LIBRARY_FILE = "library.json"
SEMVER_REGEX = r"^\d+\.\d+\.\d+$"

# ---- utility git ----
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
    return tag.lstrip("v") if tag else "0.0.0"

# ---- semver ----
def validate(version):
    if not re.match(SEMVER_REGEX, version):
        raise ValueError("Formato versione non valido (MAJOR.MINOR.PATCH)")

def bump(version, level):
    major, minor, patch = map(int, version.split("."))
    if level == "patch": patch += 1
    elif level == "minor": minor += 1; patch = 0
    elif level == "major": major += 1; minor = 0; patch = 0
    return f"{major}.{minor}.{patch}"

# ---- update library.json ----
def update_library(version, execute):
    path = Path(LIBRARY_FILE)
    if not path.exists():
        print("⚠ library.json non trovato, skip.")
        return False
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    old_version = data.get("version", "N/A")

    if execute:
        data["version"] = version
        with open(path, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2)
            f.write("\n")
        print(f"library.json aggiornato: {old_version} → {version}")
    else:
        # Dry-run: preview colorata
        RED = "\033[91m"
        GREEN = "\033[92m"
        RESET = "\033[0m"
        print("library.json preview (dry-run):")
        print(f"- {RED}{old_version}{RESET}")
        print(f"+ {GREEN}{version}{RESET}")

    return True

# ---- changelog avanzato ----
def generate_changelog(version, execute):
    changelog_path = Path("CHANGELOG.md")
    last_tag = "0.0.0"

    # trova ultimo tag nel changelog
    if changelog_path.exists():
        with open(changelog_path, "r", encoding="utf-8") as f:
            for line in f:
                if line.startswith("## v"):
                    last_tag = line.split()[1].lstrip("v")
                    break
    else:
        last_tag = get_last_tag()

    log_range = f"{last_tag}..HEAD" if last_tag else "HEAD"

    # prendi commit reali (no merge) e filtra release/chore/docs
    commits_raw = git_output(f'git log {log_range} --pretty=format:"%s" --no-merges')
    if not commits_raw:
        print("⚠ Nessun commit rilevante per il changelog.")
        return False

    features, fixes = [], []
    for c in commits_raw.splitlines():
        lc = c.lower()
        if lc.startswith("feat:"):
            features.append(c[5:].strip())
        elif lc.startswith("fix:"):
            fixes.append(c[4:].strip())
        elif lc.startswith("chore:") or lc.startswith("docs:") or lc.startswith("release"):
            continue
        else:
            fixes.append(c.strip())

    if not features and not fixes:
        print("⚠ Nessun commit rilevante dopo filtro, skip changelog.")
        return False

    today = datetime.now().strftime("%Y-%m-%d")
    new_section = f"## v{version} - {today}\n\n"

    if features:
        new_section += "### Features\n" + "\n".join(f"- {f}" for f in features) + "\n\n"
    if fixes:
        new_section += "### Fixes\n" + "\n".join(f"- {f}" for f in fixes) + "\n\n"

    if not execute:
        # preview colorata
        GREEN = "\033[92m"
        RESET = "\033[0m"
        print("\nCHANGELOG.md preview (dry-run):\n")
        for line in new_section.splitlines():
            print(f"{GREEN}{line}{RESET}")
    else:
        old_content = changelog_path.read_text(encoding="utf-8") if changelog_path.exists() else ""
        changelog_path.write_text(new_section + old_content, encoding="utf-8")
        print("CHANGELOG.md aggiornato.")

    return True

# ---- main ----
def main():
    parser = argparse.ArgumentParser(description="Release tool")
    parser.add_argument("version", nargs="?", help="Versione manuale (es 1.2.3)")
    parser.add_argument("--patch", action="store_true")
    parser.add_argument("--minor", action="store_true")
    parser.add_argument("--major", action="store_true")
    parser.add_argument("--commit", action="store_true")
    parser.add_argument("--tag", action="store_true")
    parser.add_argument("--push", action="store_true")
    parser.add_argument("--changelog", action="store_true", help="Aggiorna CHANGELOG.md")
    parser.add_argument("--go", action="store_true", help="Esegui realmente i comandi")
    args = parser.parse_args()

    # ---- determina versione ----
    if args.version:
        version = args.version.lstrip("v")
    elif args.patch or args.minor or args.major:
        level = "patch" if args.patch else "minor" if args.minor else "major"
        last = get_last_tag()
        version = bump(last, level)
        print(f"Ultimo tag: {last} → Nuova versione: {version}")
    else:
        print("Specifica versione o usa --patch/--minor/--major")
        sys.exit(1)
    validate(version)
    tag_version = f"v{version}"

    print("\n=== RELEASE PLAN ===")

    # ---- library.json ----
    if args.tag:
        updated = update_library(version, args.go)
        if updated and args.commit:
            run("git add library.json", args.go)

    # ---- changelog ----
    if args.changelog:
        updated_changelog = generate_changelog(version, args.go)
        if updated_changelog and args.commit:
            run("git add CHANGELOG.md", args.go)

    # ---- commit ----
    if args.commit:
        run(f'git commit -m "Release {version}"', args.go)

    # ---- tag ----
    if args.tag:
        run(f'git tag -a {tag_version} -m "Release {version}"', args.go)

    # ---- push ----
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
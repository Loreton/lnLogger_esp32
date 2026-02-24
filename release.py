#!/usr/bin/env python3

import json
import sys
import re
from pathlib import Path

LIBRARY_FILE = "library.json"

SEMVER_REGEX = r"^\d+\.\d+\.\d+$"


def validate_version(version):
    if not re.match(SEMVER_REGEX, version):
        raise ValueError(
            "Versione non valida. Usa formato semver: MAJOR.MINOR.PATCH (es. 1.2.3)"
        )


def update_version(new_version):
    validate_version(new_version)

    path = Path(LIBRARY_FILE)

    if not path.exists():
        raise FileNotFoundError(f"{LIBRARY_FILE} non trovato")

    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)

    old_version = data.get("version", "N/A")
    data["version"] = new_version

    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2)
        f.write("\n")

    print(f"Versione aggiornata: {old_version} → {new_version}")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Uso: python bump_version.py 1.2.3")
        sys.exit(1)

    version = sys.argv[1].lstrip("v")  # permette tag tipo v1.2.3
    update_version(version)
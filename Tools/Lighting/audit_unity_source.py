"""Read a Unity package without importing or executing its contents."""
import argparse
import collections
import json
from pathlib import Path, PurePosixPath
import re
import tarfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("package", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    paths = {}
    # Two sequential passes avoid repeated gzip seeks through a large package.
    with tarfile.open(args.package, "r|gz") as archive:
        for member in archive:
            if member.isfile() and member.name.endswith("/pathname"):
                paths[member.name.split("/")[0]] = archive.extractfile(member).read().decode("utf-8-sig").strip()
    by_guid = {guid: {"guid": guid, "path": path, "bytes": 0} for guid, path in paths.items()}
    with tarfile.open(args.package, "r|gz") as archive:
        for member in archive:
            guid, _, filename = member.name.partition("/")
            if not member.isfile() or guid not in by_guid:
                continue
            record = by_guid[guid]
            suffix = PurePosixPath(record["path"]).suffix.lower()
            if filename == "asset":
                record["bytes"] = member.size
            if filename == "asset" and suffix in (".mat", ".shader", ".txt"):
                source = archive.extractfile(member).read().decode("utf-8-sig", errors="replace")
                record["source"] = source
                record["resolved_guids"] = {g: paths.get(g, "external/builtin") for g in set(re.findall(r"guid: ([0-9a-f]{32})", source))}
            if suffix == ".fbx" and filename == "asset.meta":
                record["import_settings"] = archive.extractfile(member).read().decode("utf-8-sig", errors="replace")
    records = sorted(by_guid.values(), key=lambda record: record["path"])
    result = {"package": str(args.package), "counts": dict(collections.Counter(PurePosixPath(p).suffix.lower() or "directory" for p in paths.values())), "assets": records}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2), encoding="utf-8")
    print(json.dumps({"assets": len(records), "counts": result["counts"], "output": str(args.output)}))


if __name__ == "__main__":
    main()

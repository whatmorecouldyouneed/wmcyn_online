"""Extract explicitly requested reference assets from an audited Unity package."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import tarfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("audit", type=Path)
parser.add_argument("--output", type=Path, required=True)
parser.add_argument("--asset", action="append", required=True)
args = parser.parse_args()
audit = json.loads(args.audit.read_text(encoding="utf-8"))
wanted = set(args.asset)
selected = {a["guid"] + "/asset": a["path"] for a in audit["assets"] if a["path"] in wanted}
if len(selected) != len(wanted):
    raise ValueError("One or more requested assets were not found")
root = args.output.resolve()
extracted = []
with tarfile.open(audit["package"], "r|gz") as archive:
    for member in archive:
        if member.name not in selected or not member.isfile():
            continue
        relative = PurePosixPath(selected[member.name])
        target = root.joinpath(*relative.parts).resolve()
        if not target.is_relative_to(root):
            raise ValueError("Asset path escapes reference directory")
        data = archive.extractfile(member).read()
        if target.exists() and target.read_bytes() != data:
            raise ValueError("Refusing to overwrite a different reference file: " + str(target))
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
        extracted.append({"path": str(target), "sha256": hashlib.sha256(data).hexdigest()})
if len(extracted) != len(wanted):
    raise ValueError("Package is missing requested asset payloads")
print(json.dumps(extracted, indent=2))

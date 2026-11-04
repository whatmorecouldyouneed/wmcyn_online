"""Read-only Unity furniture dependency and original-package comparison audit."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import tarfile
import yaml


def documents(text):
    # Unity's per-document class tags are not standard YAML constructors.
    clean = re.sub(r'^%.*\n', '', text, flags=re.M)
    clean = re.sub(r'^--- !u!\d+ &-?\d+(?: stripped)?', '---', clean, flags=re.M)
    return [d for d in yaml.safe_load_all(clean) if isinstance(d, dict)]


def refs(value):
    if isinstance(value, dict):
        if value.get('guid'):
            yield value['guid']
        for child in value.values():
            yield from refs(child)
    elif isinstance(value, list):
        for child in value:
            yield from refs(child)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--project', type=Path, required=True)
    parser.add_argument('--inventory', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    inventory = json.loads(args.inventory.read_text())
    source = args.project / 'Assets/Hidden Heights SGB'
    guid_paths = {}
    for meta in (args.project / 'Assets').rglob('*.meta'):
        match = re.search(r'^guid: ([0-9a-f]{32})$', meta.read_text(errors='replace'), re.M)
        if match:
            guid_paths[match[1]] = str(meta.with_suffix('').relative_to(args.project)).replace('\\', '/')
    by_guid = {a['guid']: a for a in inventory['assets']}
    comparisons = []
    with tarfile.open(inventory['package'], 'r|gz') as archive:
        for member in archive:
            guid, _, name = member.name.partition('/')
            if name != 'asset' or guid not in by_guid or not member.isfile():
                continue
            path = by_guid[guid]['path']
            local = args.project / path
            original_hash = hashlib.sha256(archive.extractfile(member).read()).hexdigest()
            status = 'missing'
            if local.is_file():
                status = 'identical' if hashlib.sha256(local.read_bytes()).hexdigest() == original_hash else 'changed'
            comparisons.append(dict(path=path, status=status))
    def dependency_path(guid):
        return guid_paths.get(str(guid), 'UNRESOLVED:' + str(guid))
    prefabs = []
    for path in sorted((source / 'prefabs').glob('*.prefab')):
        docs = documents(path.read_text(encoding='utf-8-sig'))
        dependencies = sorted({dependency_path(g) for d in docs for g in refs(d)})
        materials = []
        for dep in dependencies:
            if dep.endswith('.mat'):
                mat_docs = documents((args.project / dep).read_text(encoding='utf-8-sig'))
                materials.append(dict(path=dep, dependencies=sorted({dependency_path(g) for d in mat_docs for g in refs(d)})))
        prefabs.append(dict(path=str(path.relative_to(args.project)).replace('\\', '/'),
            dependencies=dependencies, materials=materials,
            names=[d['GameObject'].get('m_Name') for d in docs if 'GameObject' in d],
            transforms=[{k:v for k,v in d['Transform'].items() if k in ('m_LocalPosition','m_LocalRotation','m_LocalScale','m_Father','m_GameObject')} for d in docs if 'Transform' in d],
            colliders=[d for d in docs if any('Collider' in k for k in d)]))
    scenes = []
    for path in sorted((source / 'Scenes').glob('*.unity')):
        docs = documents(path.read_text(encoding='utf-8-sig'))
        counts = Counter()
        for d in docs:
            instance = d.get('PrefabInstance', {})
            guid = instance.get('m_SourcePrefab', {}).get('guid')
            if guid:
                counts[dependency_path(guid)] += 1
        scenes.append(dict(path=str(path.relative_to(args.project)).replace('\\','/'),
                           objects=len(docs), prefab_instances=dict(counts)))
    report = dict(project=str(args.project), counts=dict(Counter(c['status'] for c in comparisons)),
                  comparisons=comparisons, prefabs=prefabs, scenes=scenes,
                  limitations=['Transforms are prefab-local, not calibrated Unreal placements.',
                               'No Unity scripts executed. No Unreal assets modified.',
                               'Unresolved GUIDs may be builtins or external package dependencies.'])
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2))
    print(json.dumps(dict(counts=report['counts'], prefabs=len(prefabs),
                         scenes=[dict(path=s['path'], objects=s['objects']) for s in scenes])))


if __name__ == '__main__':
    main()

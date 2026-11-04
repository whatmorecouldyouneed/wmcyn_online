"""Read-only dependency and material audit of the user-approved Jared output."""
import json
from pathlib import Path
import unreal

registry = unreal.AssetRegistryHelpers.get_asset_registry()
root = '/Game/MetaHumans/Jared'
assets = registry.get_assets_by_path(root, recursive=True)
options = unreal.AssetRegistryDependencyOptions(
    include_soft_package_references=True, include_hard_package_references=True,
    include_searchable_names=False, include_soft_management_references=False,
    include_hard_management_references=False)
report = {'root': root, 'asset_count': len(assets), 'missing_dependencies': [],
          'skeletal_meshes': [], 'material_instances': [], 'errors': []}
pending = [str(a.package_name) for a in assets]
seen = set()
while pending:
    package = pending.pop()
    if package in seen:
        continue
    seen.add(package)
    for dep in registry.get_dependencies(package, options):
        dep = str(dep)
        if not dep.startswith('/Game/'):
            continue
        if not registry.get_assets_by_package_name(dep):
            report['missing_dependencies'].append({'from': package, 'missing': dep})
        elif dep not in seen:
            pending.append(dep)
for data in assets:
    cls = str(data.asset_class_path.asset_name)
    if cls not in ('SkeletalMesh', 'MaterialInstanceConstant'):
        continue
    try:
        obj = data.get_asset()
        if cls == 'SkeletalMesh':
            report['skeletal_meshes'].append({
                'path': obj.get_path_name(),
                'skeleton': str(obj.get_editor_property('skeleton')),
                'materials': [{'slot': str(m.material_slot_name),
                               'material': m.material_interface.get_path_name() if m.material_interface else None}
                              for m in obj.get_editor_property('materials')]})
        else:
            parent = obj.get_editor_property('parent')
            report['material_instances'].append({
                'path': obj.get_path_name(),
                'parent': parent.get_path_name() if parent else None,
                'textures': [{'parameter': str(t.parameter_info.name),
                              'texture': t.parameter_value.get_path_name() if t.parameter_value else None}
                             for t in obj.get_editor_property('texture_parameter_values')]})
    except Exception as exc:
        report['errors'].append({'asset': str(data.package_name), 'error': str(exc)})
report['packages_checked'] = len(seen)
out = Path(unreal.Paths.project_saved_dir()) / 'Diagnostics' / 'Avatars'
out.mkdir(parents=True, exist_ok=True)
(out / 'jared_asset_audit.json').write_text(json.dumps(report, indent=2))
print('WMCYN_JARED_AUDIT', len(assets), 'assets;', len(seen), 'packages;',
      len(report['missing_dependencies']), 'missing dependencies;', len(report['errors']), 'errors')

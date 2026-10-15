"""Wire per-leaf grip anchors using the engine's Blueprint DSL tool API."""
import json
from pathlib import Path
import unreal
from editor_toolset.toolsets.blueprint import BlueprintTools

root = Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting'
source = (Path(__file__).parent/'crib_door_01.dsl').read_text()
for item in json.loads((root/'door_rollout_created.json').read_text()):
    path = item['blueprint']
    name = path.rsplit('/',1)[1]
    bp = unreal.load_asset(path)
    graph = unreal.load_object(None,path+'.'+name+':EventGraph')
    assert graph
    anchor = ','.join(f'{x:.4f}' for x in item['handle'])
    code = source.replace('"23.5,-87,110.4"','"'+anchor+'"').replace('Door 01',f"Door {item['number']:02d}")
    code = code.replace('(and down (not (Variables|Default|GetGripWasDown)))', '(and down (and (< distance 35.0) (not (Variables|Default|GetGripWasDown))))')
    BlueprintTools.write_graph_dsl(graph,code)
    BlueprintTools.compile_blueprint(bp,warnings_as_errors=True)
    unreal.EditorAssetLibrary.save_loaded_asset(bp)
    print('WMCYN_DOOR_WIRED',item['number'])
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()

import unreal
from pathlib import Path
paths={
 'video':'/Game/AFCore/Blueprints/Components/Misc/Comp_Video.Comp_Video_C',
 'widget':'/Game/AFCore/Blueprints/Components/UI/Comp_Widget.Comp_Widget_C',
 'keyboard':'/Game/AFCore/Blueprints/UI/Overlay/Blueprints/BP_Overlay_Widget_Keyboard.BP_Overlay_Widget_Keyboard_C',
 'login':'/Game/WMCYN/UI/WBP_WMCYN_LoginJoin.WBP_WMCYN_LoginJoin_C',
 'base':'/Game/AFCore/Blueprints/Widgets/Core/Widget_Base.Widget_Base_C'}
out=Path(unreal.Paths.project_saved_dir())/'Diagnostics/Video'
out.mkdir(parents=True,exist_ok=True)
for key,path in paths.items():
    (out/(key+'.txt')).write_text(unreal.WMCYNVideoEditorLibrary.inspect_class(path))

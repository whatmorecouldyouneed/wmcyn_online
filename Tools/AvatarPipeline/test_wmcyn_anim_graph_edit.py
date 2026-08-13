import json
from pathlib import Path

import unreal


def pin_label(pin):
    for attr in ("pin_name", "name"):
        try:
            return str(pin.get_editor_property(attr))
        except Exception:
            pass
    return str(pin)


def main():
    report = {}
    anim_blueprint = unreal.load_asset(
        "/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/ABP_WMCYN_MetaHuman_Presentation"
    )
    graph = anim_blueprint.get_animation_graphs()[0]
    report["graph"] = graph.get_path_name()
    report["before"] = [node.get_class().get_name() for node in graph.get_graph_nodes_of_class(unreal.AnimGraphNode_Root)]
    node = unreal.new_object(unreal.AnimGraphNode_RetargetPoseFromMesh, outer=graph)
    report["node"] = node.get_path_name() if node else ""
    report["node_pins"] = [pin_label(pin) for pin in node.list_all_pins()] if node else []
    roots = graph.get_graph_nodes_of_class(unreal.AnimGraphNode_Root)
    report["root_pins"] = [pin_label(pin) for pin in roots[0].list_all_pins()] if roots else []
    try:
        result = graph.call_method("AddNode", node)
        report["call_method"] = str(result)
    except Exception as exc:
        report["call_method_error"] = str(exc)
    try:
        report["retarget_nodes_after_call"] = [node.get_path_name() for node in graph.get_graph_nodes_of_class(unreal.AnimGraphNode_RetargetPoseFromMesh)]
    except Exception as exc:
        report["retarget_nodes_after_call_error"] = str(exc)
    try:
        report["protected_nodes_read"] = str(graph.get_editor_property("nodes"))
    except Exception as exc:
        report["protected_nodes_read_error"] = str(exc)
    try:
        graph.set_editor_property("nodes", [node])
        report["protected_nodes_write"] = "ok"
    except Exception as exc:
        report["protected_nodes_write_error"] = str(exc)
    try:
        report["retarget_nodes_after_write"] = [node.get_path_name() for node in graph.get_graph_nodes_of_class(unreal.AnimGraphNode_RetargetPoseFromMesh)]
    except Exception as exc:
        report["retarget_nodes_after_write_error"] = str(exc)
    output = Path(unreal.Paths.project_dir()) / "Saved/AvatarPipeline/Results/wmcyn_anim_graph_edit_test.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2), encoding="utf-8")
    unreal.log(f"WMCYN transient animation graph test written to {output}")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        output = Path(unreal.Paths.project_dir()) / "Saved/AvatarPipeline/Results/wmcyn_anim_graph_edit_test.json"
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(json.dumps({"fatal_error": repr(exc)}, indent=2), encoding="utf-8")
        unreal.log_error(f"WMCYN transient animation graph test failed: {exc}")

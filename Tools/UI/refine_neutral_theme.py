"""Refine the WMCYN-owned AFCore theme maps; never write framework assets."""
import unreal

PATH = '/Game/WMCYN/UI/Theme/DA_WMCYN_Theme_Neutral'
theme = unreal.load_asset(PATH)
assert theme is not None
fonts = theme.get_editor_property('font')
for key in list(fonts.keys()):
    font = fonts[key]
    font.set_editor_property('size', 28 if 'DISPLAY' in str(key).upper() else 24)
    font.set_editor_property('typeface_font_name', 'Regular')
    font.set_editor_property('letter_spacing', 0)
    fonts[key] = font
theme.set_editor_property('font', fonts)
for property_name, value in [('categoryToCornerWidth', 8.0), ('categoryToOutlineWidth', 1.0)]:
    values = theme.get_editor_property(property_name)
    for key in list(values.keys()):
        values[key] = value
    theme.set_editor_property(property_name, values)
unreal.EditorAssetLibrary.save_asset(PATH, only_if_is_dirty=False)
print('WMCYN neutral theme maps saved', theme.get_editor_property('font'))

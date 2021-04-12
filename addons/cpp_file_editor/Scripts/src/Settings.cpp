#include <Godot.hpp>
#include <Control.hpp>
#include <ConfigFile.hpp>
#include <TreeItem.hpp>
#include <FileDialog.hpp>
#include <WindowDialog.hpp>
#include <PoolArrays.hpp>
#include <Texture.hpp>
#include <Reference.hpp>
#include <ResourceLoader.hpp>
#include <LineEdit.hpp>
#include <SpinBox.hpp>
#include <OptionButton.hpp>
#include <CheckButton.hpp>
#include <OS.hpp>
#include <ColorPickerButton.hpp>
#include <ItemList.hpp>

#include "Settings.hpp"
#include "FileManager.hpp"
using namespace godot;

Settings::Settings()
{
}

Settings::~Settings()
{
}

void Settings::_init()
{
}

void Settings::_ready()
{
    tree = ((Tree *)get_node(NodePath("VBoxContainer/Settings/CategoryTree")));

    TreeItem *root = tree->create_item();
    TreeItem *general = tree->create_item(root);
    general->set_text(0, "General");
    general->set_selectable(0, false);

    TreeItem *editor = tree->create_item(general);
    editor->set_text(0, "Editor");
    editor->select(0);

    TreeItem *languages = tree->create_item(root);
    languages->set_text(0, "Languages");
    languages->set_selectable(0, false);

    TreeItem *cpp = tree->create_item(languages);
    Ref<Texture> icon;
    icon = ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Icons/cplusplus-original.svg", "Texture");
    cpp->set_icon(0, icon);
    cpp->set_text(0, "C++");

    TreeItem *rust = tree->create_item(languages);
    icon = ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Icons/rust-plain.svg", "Texture");
    rust->set_icon(0, icon);
    rust->set_text(0, "Rust");

    tree->set_hide_root(true);
    this->set_editor_data();
    this->set_cpp_data();
    this->set_rust_data();
}

void Settings::show_window()
{
    cast_to<WindowDialog>(this)->popup_centered();
}

void Settings::set_editor_data()
{
    PoolStringArray keys = Array::make("custom_font", "custom_theme");
    PoolStringArray settings = cast_to<EditorFile>(this->get_parent())->load_config("user://editor.cfg", "Editor", keys);
    keys = Array::make("font_size");
    int font_size = cast_to<EditorFile>(this->get_parent())->load_config("user://editor.cfg", "Editor", keys)[0];

    ((LineEdit *)get_node(NodePath("VBoxContainer/Settings/EditorTree/General/FontName/LineEdit")))->set_text(settings[0]);
    ((SpinBox *)get_node(NodePath("VBoxContainer/Settings/EditorTree/General/FontSize/SpinBox")))->set_value(font_size);
    ((LineEdit *)get_node(NodePath("VBoxContainer/Settings/EditorTree/General/CustomTheme/LineEdit")))->set_text(settings[1]);

    keys = Array::make("exit_success", "exit_error");
    PoolColorArray global_colors = cast_to<EditorFile>(this->get_parent())->load_config("user://syntax.cfg", "Global", keys);
    ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/EditorTree/Syntax/ExitSuccess/ColorPickerButton")))->set_pick_color(global_colors[0]);
    ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/EditorTree/Syntax/ExitError/ColorPickerButton")))->set_pick_color(global_colors[1]);
}

void Settings::set_cpp_data()
{
    PoolStringArray keys = Array::make("cpp_standard", "optimization");
    PoolIntArray settings = cast_to<EditorFile>(this->get_parent())->load_config("user://editor.cfg", "C++", keys);
    keys = Array::make("global_build", "global_clean");
    PoolStringArray global_build = cast_to<EditorFile>(this->get_parent())->load_config("user://editor.cfg", "C++", keys);

    ((OptionButton *)get_node(NodePath("VBoxContainer/Settings/CPPTree/General/Standard/OptionButton")))->select(settings[0]);
    ((OptionButton *)get_node(NodePath("VBoxContainer/Settings/CPPTree/General/Optimization/OptionButton")))->select(settings[1]);
    ((LineEdit *)get_node(NodePath("VBoxContainer/Settings/CPPTree/General/GlobalBuild/LineEdit")))->set_text(global_build[0]);
    ((LineEdit *)get_node(NodePath("VBoxContainer/Settings/CPPTree/General/GlobalClean/LineEdit")))->set_text(global_build[1]);

    keys = Array::make("functions", "strings", "comments", "preprocessor", "keywords");
    PoolColorArray cpp_colors = cast_to<EditorFile>(this->get_parent())->load_config("user://syntax.cfg", "C++", keys);
    ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/CPPTree/Syntax/Functions/ColorPickerButton")))->set_pick_color(cpp_colors[0]);
    ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/CPPTree/Syntax/Strings/ColorPickerButton")))->set_pick_color(cpp_colors[1]);
    ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/CPPTree/Syntax/Comments/ColorPickerButton")))->set_pick_color(cpp_colors[2]);
    ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/CPPTree/Syntax/Preprocessor/ColorPickerButton")))->set_pick_color(cpp_colors[3]);
    ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/CPPTree/Syntax/Keywords/ColorPickerButton")))->set_pick_color(cpp_colors[4]);
}

void Settings::set_rust_data()
{
    PoolStringArray args;
    args.append("--version");
    Array output;
    String cargo_version = OS::get_singleton()->execute("cargo", args, true, output);
    
    PoolStringArray keys = Array::make("check_on_save", "pass_target_all", "pass_offline");
    bool cargo_check = cast_to<EditorFile>(this->get_parent())->load_config("user://editor.cfg", "Rust", keys)[0];
    bool target_all = cast_to<EditorFile>(this->get_parent())->load_config("user://editor.cfg", "Rust", keys)[1];
    bool offline = cast_to<EditorFile>(this->get_parent())->load_config("user://editor.cfg", "Rust", keys)[2];
    ((LineEdit *)get_node(NodePath("VBoxContainer/Settings/RustTree/General/General/CargoVersion/LineEdit")))->set_text(output[0]);
    ((CheckButton *)get_node(NodePath("VBoxContainer/Settings/RustTree/General/General/CargoCheck/CheckButton")))->set_pressed(cargo_check);
    ((CheckButton *)get_node(NodePath("VBoxContainer/Settings/RustTree/General/General/PassTargetAll/CheckButton")))->set_pressed(target_all);
    ((CheckButton *)get_node(NodePath("VBoxContainer/Settings/RustTree/General/General/PassOffline/CheckButton")))->set_pressed(offline);

    args.remove(0);
    output.clear();
    args.append("toolchain");
    args.append("list");
    OS::get_singleton()->execute("rustup", args, true, output);
    for (int i = 0; i < output.size(); i++)
    {
        ((ItemList *)get_node(NodePath("VBoxContainer/Settings/RustTree/General/General/Toolchains/ItemList")))->add_item(output[i]);
    }

    keys = Array::make("global_build", "global_clean");
    PoolStringArray global_options = cast_to<EditorFile>(this->get_parent())->load_config("user://editor.cfg", "Rust", keys);
    ((LineEdit *)get_node(NodePath("VBoxContainer/Settings/RustTree/General/General/GlobalBuild/LineEdit")))->set_text(global_options[0]);
    ((LineEdit *)get_node(NodePath("VBoxContainer/Settings/RustTree/General/General/GlobalClean/LineEdit")))->set_text(global_options[1]);

    keys = Array::make("functions", "strings", "comments", "keywords", "types");
    PoolColorArray rust_colors = cast_to<EditorFile>(this->get_parent())->load_config("user://syntax.cfg", "Rust", keys);
    ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/RustTree/Syntax/Functions/ColorPickerButton")))->set_pick_color(rust_colors[0]);
    ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/RustTree/Syntax/Strings/ColorPickerButton")))->set_pick_color(rust_colors[1]);
    ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/RustTree/Syntax/Comments/ColorPickerButton")))->set_pick_color(rust_colors[2]);
    ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/RustTree/Syntax/Keywords/ColorPickerButton")))->set_pick_color(rust_colors[3]);
    ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/RustTree/Syntax/Types/ColorPickerButton")))->set_pick_color(rust_colors[4]);
}

void Settings::save_editor_data()
{
    ConfigFile *config_file = ConfigFile::_new();
    config_file->load("user://editor.cfg");

    String custom_font = ((LineEdit *)get_node(NodePath("VBoxContainer/Settings/EditorTree/General/FontName/LineEdit")))->get_text();
    config_file->set_value("Editor", "custom_font", custom_font);

    int font_size = ((SpinBox *)get_node(NodePath("VBoxContainer/Settings/EditorTree/General/FontSize/SpinBox")))->get_value();
    config_file->set_value("Editor", "font_size", font_size);

    String custom_theme = ((LineEdit *)get_node(NodePath("VBoxContainer/Settings/EditorTree/General/CustomTheme/LineEdit")))->get_text();
    config_file->set_value("Editor", "custom_theme", custom_theme);

    config_file->save("user://editor.cfg");
    config_file->free();

    config_file = ConfigFile::_new();
    config_file->load("user://syntax.cfg");

    Color exit_success = ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/EditorTree/Syntax/ExitSuccess/ColorPickerButton")))->get_pick_color();
    config_file->set_value("Global", "exit_success", exit_success);

    Color exit_error = ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/EditorTree/Syntax/ExitError/ColorPickerButton")))->get_pick_color();
    config_file->set_value("Global", "exit_error", exit_error);

    config_file->save("user://syntax.cfg");
    config_file->free();

    cast_to<EditorFile>(this->get_parent())->load_editor_settings();
}

void Settings::save_cpp_data()
{
    ConfigFile *config_file = ConfigFile::_new();
    config_file->load("user://editor.cfg");

    int cpp_standard = ((OptionButton *)get_node(NodePath("VBoxContainer/Settings/CPPTree/General/Standard/OptionButton")))->get_selected();
    config_file->set_value("C++", "cpp_standard", cpp_standard);

    int optimization = ((OptionButton *)get_node(NodePath("VBoxContainer/Settings/CPPTree/General/Optimization/OptionButton")))->get_selected();
    config_file->set_value("C++", "optimization", optimization);

    String global_build = ((LineEdit *)get_node(NodePath("VBoxContainer/Settings/CPPTree/General/GlobalBuild/LineEdit")))->get_text();
    config_file->set_value("C++", "global_build", global_build);

    String global_clean = ((LineEdit *)get_node(NodePath("VBoxContainer/Settings/CPPTree/General/GlobalClean/LineEdit")))->get_text();
    config_file->set_value("C++", "global_clean", global_clean);

    config_file->save("user://editor.cfg");
    config_file->free();
    
    config_file = ConfigFile::_new();
    config_file->load("user://syntax.cfg");

    Color functions = ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/CPPTree/Syntax/Functions/ColorPickerButton")))->get_pick_color();
    config_file->set_value("C++", "functions", functions);
    
    Color strings = ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/CPPTree/Syntax/Strings/ColorPickerButton")))->get_pick_color();
    config_file->set_value("C++", "strings", strings);
    
    Color comments = ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/CPPTree/Syntax/Comments/ColorPickerButton")))->get_pick_color();
    config_file->set_value("C++", "comments", comments);
    
    Color preprocessor = ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/CPPTree/Syntax/Preprocessor/ColorPickerButton")))->get_pick_color();
    config_file->set_value("C++", "preprocessor", preprocessor);
    
    Color keywords = ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/CPPTree/Syntax/Keywords/ColorPickerButton")))->get_pick_color();
    config_file->set_value("C++", "keywords", keywords);

    config_file->save("user://syntax.cfg");
    config_file->free();

    cast_to<EditorFile>(this->get_parent())->load_color_settings();
}

void Settings::save_rust_data()
{
    ConfigFile *config_file = ConfigFile::_new();
    config_file->load("user://editor.cfg");

    bool cargo_check = ((CheckButton *)get_node(NodePath("VBoxContainer/Settings/RustTree/General/General/CargoCheck/CheckButton")))->is_pressed();
    config_file->set_value("Rust", "check_on_save", cargo_check);
    
    bool target_all = ((CheckButton *)get_node(NodePath("VBoxContainer/Settings/RustTree/General/General/PassTargetAll/CheckButton")))->is_pressed();
    config_file->set_value("Rust", "pass_target_all", target_all);

    bool offline = ((CheckButton *)get_node(NodePath("VBoxContainer/Settings/RustTree/General/General/PassOffline/CheckButton")))->is_pressed();
    config_file->set_value("Rust", "pass_offline", offline);

    String global_build = ((LineEdit *)get_node(NodePath("VBoxContainer/Settings/RustTree/General/General/GlobalBuild/LineEdit")))->get_text();
    config_file->set_value("Rust", "global_build", global_build);

    String global_clean = ((LineEdit *)get_node(NodePath("VBoxContainer/Settings/RustTree/General/General/GlobalClean/LineEdit")))->get_text();
    config_file->set_value("Rust", "global_clean", global_clean);

    config_file->save("user://editor.cfg");
    config_file->free();
    
    config_file = ConfigFile::_new();
    config_file->load("user://syntax.cfg");

    Color functions = ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/RustTree/Syntax/Functions/ColorPickerButton")))->get_pick_color();
    config_file->set_value("Rust", "functions", functions);
    
    Color strings = ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/RustTree/Syntax/Strings/ColorPickerButton")))->get_pick_color();
    config_file->set_value("Rust", "strings", strings);
    
    Color comments = ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/RustTree/Syntax/Comments/ColorPickerButton")))->get_pick_color();
    config_file->set_value("Rust", "comments", comments);

    Color keywords = ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/RustTree/Syntax/Keywords/ColorPickerButton")))->get_pick_color();
    config_file->set_value("Rust", "keywords", keywords);
    
    Color types = ((ColorPickerButton *)get_node(NodePath("VBoxContainer/Settings/RustTree/Syntax/Types/ColorPickerButton")))->get_pick_color();
    config_file->set_value("Rust", "types", types);

    config_file->save("user://syntax.cfg");
    config_file->free();
    
    cast_to<EditorFile>(this->get_parent())->load_color_settings();
}

void Settings::_on_ConfirmSettings_pressed()
{
    String selected = ((Tree *)get_node(NodePath("VBoxContainer/Settings/CategoryTree")))->get_selected()->get_text(0);

    if (selected == "Editor")
    {
        this->save_editor_data();
    }
    else if (selected == "C++")
    {
        this->save_cpp_data();
    }
    else if (selected == "Rust")
    {
        this->save_rust_data();
    }

    this->hide();
}

void Settings::_on_SearchFont_pressed()
{
    selected_id = 0;
    PoolStringArray filter;
    filter.append("*.ttf");
    filter.append("*.otf");
    ((FileDialog *)get_node(NodePath("OpenFile")))->set_filters(filter);
    ((FileDialog *)get_node(NodePath("OpenFile")))->popup_centered();
}

void Settings::_on_SearchTheme_pressed()
{
    selected_id = 1;
    PoolStringArray filter;
    filter.append("*.tres");
    ((FileDialog *)get_node(NodePath("OpenFile")))->set_filters(filter);
    ((FileDialog *)get_node(NodePath("OpenFile")))->popup_centered();
}

void Settings::_on_OpenFile_file_selected(String path)
{
    switch (selected_id)
    {
    case 0:
        ((LineEdit *)get_node(NodePath("VBoxContainer/Settings/EditorTree/General/FontName/LineEdit")))->set_text(path);
        break;
    case 1:
        
        ((LineEdit *)get_node(NodePath("VBoxContainer/Settings/EditorTree/General/CustomTheme/LineEdit")))->set_text(path);
        break;
    }
    selected_id = -1;
}

void Settings::_on_CategoryTree_item_selected()
{
    String selected = ((Tree *)get_node(NodePath("VBoxContainer/Settings/CategoryTree")))->get_selected()->get_text(0);

    if (selected == "Editor")
    {
        ((Control *)get_node(NodePath("VBoxContainer/Settings/CPPTree")))->hide();
        ((Control *)get_node(NodePath("VBoxContainer/Settings/RustTree")))->hide();
        ((Control *)get_node(NodePath("VBoxContainer/Settings/EditorTree")))->show();
    }
    else if (selected == "C++")
    {
        ((Control *)get_node(NodePath("VBoxContainer/Settings/RustTree")))->hide();
        ((Control *)get_node(NodePath("VBoxContainer/Settings/EditorTree")))->hide();
        ((Control *)get_node(NodePath("VBoxContainer/Settings/CPPTree")))->show();
    }
    else if (selected == "Rust")
    {
        ((Control *)get_node(NodePath("VBoxContainer/Settings/EditorTree")))->hide();
        ((Control *)get_node(NodePath("VBoxContainer/Settings/CPPTree")))->hide();
        ((Control *)get_node(NodePath("VBoxContainer/Settings/RustTree")))->show();
    }
}

void Settings::_register_methods()
{
    register_method((char *)"_init", &Settings::_init);
    register_method((char *)"_ready", &Settings::_ready);
    register_method((char *)"show", &Settings::show_window);
    register_method((char *)"set_editor_data", &Settings::set_editor_data);
    register_method((char *)"set_cpp_data", &Settings::set_cpp_data);
    register_method((char *)"set_rust_data", &Settings::set_rust_data);
    register_method((char *)"save_editor_data", &Settings::save_editor_data);
    register_method((char *)"save_cpp_data", &Settings::save_cpp_data);
    register_method((char *)"save_rust_data", &Settings::save_rust_data);

    register_method((char *)"_on_ConfirmSettings_pressed", &Settings::_on_ConfirmSettings_pressed);
    register_method((char *)"_on_SearchFont_pressed", &Settings::_on_SearchFont_pressed);
    register_method((char *)"_on_SearchTheme_pressed", &Settings::_on_SearchTheme_pressed);
    register_method((char *)"_on_OpenFile_file_selected", &Settings::_on_OpenFile_file_selected);
    register_method((char *)"_on_CategoryTree_item_selected", &Settings::_on_CategoryTree_item_selected);
}
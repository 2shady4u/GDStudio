#include <Godot.hpp>
#include <Control.hpp>
#include <stdio.h>
#include <File.hpp>
#include <FileDialog.hpp>
#include <WindowDialog.hpp>
#include <MenuButton.hpp>
#include <PopupMenu.hpp>
#include <InputEventKey.hpp>
#include <Texture.hpp>
#include <ConfigFile.hpp>
#include <Theme.hpp>
#include <Array.hpp>
#include <PoolArrays.hpp>
#include <TextEdit.hpp>
#include <Panel.hpp>
#include <OS.hpp>
#include <RichTextLabel.hpp>

#include "Sidebar.hpp"
#include "FileManager.hpp"
#include "ProjectManager.hpp"
#include "Settings.hpp"
#include "ProjectSettings.hpp"
using namespace godot;

EditorFile::EditorFile()
{
}

EditorFile::~EditorFile()
{
}

void EditorFile::_init()
{
    create_user_data();
    this->settings_scene = ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Scenes/Settings.tscn");
    this->code_scene = ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Scenes/CodeEditor.tscn");
}

void EditorFile::_ready()
{
    this->tabNode = ((Tabs *)get_node("VBoxContainer/HBoxContainer/VBoxContainer/TabContainer"));
    ((MenuButton *)get_node(NodePath("VBoxContainer/TopBar/File")))->get_popup()->connect("id_pressed", this, "on_file_pressed");
    ((MenuButton *)get_node(NodePath("VBoxContainer/TopBar/Project")))->get_popup()->connect("id_pressed", this, "on_project_pressed");
    ((MenuButton *)get_node(NodePath("VBoxContainer/TopBar/Settings")))->get_popup()->connect("id_pressed", this, "on_settings_pressed");
    create_shortcuts();
}

void EditorFile::open_file(String path)
{
    File *file = File::_new();
    file->open(path, File::READ);
    this->current_editor_instance = cast_to<CodeEditor>(this->code_scene->instance());
    this->instance_defined = true;
    String content = file->get_as_text();
    file->close();
    file->free();
    ((Tabs *)get_node("VBoxContainer/HBoxContainer/VBoxContainer/Editor"))->add_child(this->current_editor_instance, true);
    this->current_editor_instance->set_custom_font(this->custom_font);
    this->current_editor_instance->set_font_size(this->font_size);
    this->current_editor_instance->set_custom_theme(this->custom_theme);
    this->tab_number = ((Tabs *)get_node("VBoxContainer/HBoxContainer/VBoxContainer/Editor"))->get_child_count();
    this->file_path = path;

    this->file_name = path.get_file();
    String file_extension = this->file_name.get_extension();

    Ref<Texture> icon = this->get_file_icon(file_extension);

    if (file_extension == "cpp" || file_extension == "hpp")
    {
        this->current_editor_instance->setup_language("cpp");
    }
    else if (file_extension == "rs")
    {
        this->current_editor_instance->setup_language("rust");
    }
    else if (file_extension == "gdnproj")
    {
        PoolStringArray keys = Array::make("path");
        this->change_project_path(this->load_config(path, "settings", keys)[0]);
    }
    this->current_editor_instance->set_initial_content(content);
    
    ((Tabs *)get_node("VBoxContainer/HBoxContainer/VBoxContainer/TabContainer"))->add_tab(this->file_name, icon);
    ((Tabs *)get_node("VBoxContainer/HBoxContainer/VBoxContainer/TabContainer"))->set_current_tab(this->tab_number - 1);
}

void EditorFile::save_file()
{
    File *file = File::_new();
    file->open(file_path, File::WRITE);
    file->store_string(this->current_editor_instance->get_content());
    this->current_editor_instance->save_contents();
    file->close();
    file->free();

    if (this->get_project_lang() == "rust")
    {
        PoolStringArray keys = Array::make("check_on_save");
        bool check_on_save = this->load_config("user://editor.cfg", "Rust", keys)[0];
        if (check_on_save == true)
        {
            std::future<void> th = std::async(std::launch::async, &EditorFile::execute_command, this, "cargo check --manifest-path=" + this->project_config + "/Cargo.toml");
        }
    }
}

Array EditorFile::load_config(String file, String section, PoolStringArray key)
{
    ConfigFile *config_file = ConfigFile::_new();
    config_file->load(file);

    Array output;
    for (int i = 0; i < key.size(); i++)
    {
        if (config_file->has_section_key(section, key[i]) == false)
        {
            config_file->set_value(section, key[i], "");
            config_file->save(file);
        }
        output.append(config_file->get_value(section, key[i]));
    }
    config_file->free();

    return output;
}

void EditorFile::load_editor_settings()
{
    PoolStringArray keys = Array::make("custom_font", "font_size", "custom_theme");
    Array settings = this->load_config("user://editor.cfg", "Editor", keys);

    this->custom_font = settings[0];
    this->font_size = settings[1];
    this->custom_theme = settings[2];

    if (this->tab_number > 0)
    {
        for (int i = 0; i < this->tab_number; i++)
        {
            cast_to<CodeEditor>(((Tabs *)get_node("VBoxContainer/HBoxContainer/VBoxContainer/Editor"))->get_children()[i])->set_custom_font(this->custom_font);
            cast_to<CodeEditor>(((Tabs *)get_node("VBoxContainer/Editor"))->get_children()[i])->set_font_size(this->font_size);
        }
    }

    Ref<Theme> theme = ResourceLoader::get_singleton()->load(this->custom_theme);
    cast_to<Control>(this)->set_theme(theme);
}

void EditorFile::load_color_settings()
{
    if (this->tab_number > 0)
    {
        for (int i = 0; i < this->tab_number; i++)
        {
            String lang = cast_to<CodeEditor>(((Tabs *)get_node("VBoxContainer/HBoxContainer/VBoxContainer/Editor"))->get_children()[i])->get_language();
            cast_to<CodeEditor>(((Tabs *)get_node("VBoxContainer/HBoxContainer/VBoxContainer/Editor"))->get_children()[i])->setup_language(lang);
        }
    }

}

CodeEditor *EditorFile::get_editor_instance()
{
    return this->current_editor_instance;
}

String EditorFile::get_project_path()
{
    return this->project_config;
}

String EditorFile::get_selected_platform()
{
    return cast_to<Sidebar>(get_node(NodePath("VBoxContainer/HBoxContainer/Sidebar")))->get_build_platform();
}

bool EditorFile::get_selected_profile()
{
    return cast_to<Sidebar>(get_node(NodePath("VBoxContainer/HBoxContainer/Sidebar")))->get_release_flag();
}

String EditorFile::get_project_lang()
{
    if (this->project_config != "")
    {
        PoolStringArray keys = Array::make("language");
        String lang = this->load_config(this->project_config + "/settings.gdnproj", "settings", keys)[0];
        return lang;
    }
    else
    {
        return "none";
    }
}

Ref<Texture> EditorFile::get_file_icon(String extension)
{
    if (extension == "folder")
    {
        return ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Icons/default_folder.svg", "Texture");
    }
    else if (extension == "cpp")
    {
        return ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Icons/cplusplus-original.svg", "Texture");
    }
    else if (extension == "hpp")
    {
        return ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Icons/cppheader.svg", "Texture");
    }
    else if (extension == "rs")
    {
        return ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Icons/rust-plain.svg", "Texture");
    }
    else if (extension == "ini")
    {
        return ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Icons/ini.svg", "Texture");
    }
    else if (extension == "bat" || extension == "sh")
    {
        return ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Icons/shell.svg", "Texture");
    }
    else if (extension == "xml")
    {
        return ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Icons/xml.svg", "Texture");
    }
    else if (extension == "zip")
    {
        return ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Icons/zip.svg", "Texture");
    }
    else
    {
        return ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Icons/file.svg", "Texture");
    }
}

void EditorFile::create_shortcuts()
{
    Ref<InputEventKey> hotkey;

    hotkey.instance();
    hotkey->set_scancode(80);
    hotkey->set_control(true);
    ((MenuButton *)get_node(NodePath("VBoxContainer/TopBar/File")))->get_popup()->set_item_accelerator(0, hotkey->get_scancode_with_modifiers());

    hotkey->set_scancode(78);
    hotkey->set_control(true);
    ((MenuButton *)get_node(NodePath("VBoxContainer/TopBar/File")))->get_popup()->set_item_accelerator(1, hotkey->get_scancode_with_modifiers());

    hotkey->set_scancode(79);
    hotkey->set_control(true);
    ((MenuButton *)get_node(NodePath("VBoxContainer/TopBar/File")))->get_popup()->set_item_accelerator(2, hotkey->get_scancode_with_modifiers());

    hotkey->set_scancode(52);
    hotkey->set_control(true);
    ((MenuButton *)get_node(NodePath("VBoxContainer/TopBar/File")))->get_popup()->set_item_accelerator(3, hotkey->get_scancode_with_modifiers());

    hotkey->set_scancode(83);
    hotkey->set_control(true);
    ((MenuButton *)get_node(NodePath("VBoxContainer/TopBar/File")))->get_popup()->set_item_accelerator(5, hotkey->get_scancode_with_modifiers());

    hotkey->set_scancode(83);
    hotkey->set_control(true);
    hotkey->set_alt(true);
    ((MenuButton *)get_node(NodePath("VBoxContainer/TopBar/File")))->get_popup()->set_item_accelerator(6, hotkey->get_scancode_with_modifiers());
}

void EditorFile::create_user_data()
{
    File *file = File::_new();
    if (file->file_exists("user://editor.cfg") == false)
    {
        file->open("user://editor.cfg", File::WRITE);
        file->store_string("[Editor]\ncustom_font=\"res://addons/cpp_file_editor/Fonts/RobotoSlab-VariableFont_wght.ttf\"\nfont_size=12\ncustom_theme=\"res://addons/cpp_file_editor/Themes/godot_theme.tres\"\n"
                           "[C++]\ncpp_standard=0\noptimization=2\nglobal_build=\"-Q\"\nglobal_clean=\"\"\n"
                           "[Rust]\ncheck_on_save=false\npass_target_all=false\npass_offline=false\nglobal_build=\"\"\nglobal_clean=\"\"");
        file->close();
    }
    if (file->file_exists("user://syntax.cfg") == false)
    {
        file->open("user://syntax.cfg", File::WRITE);
        file->store_string("[Global]\nexit_success=Color(0,0.15,0.85,1)\nexit_error=Color(0.85,0.15,0.0,1)\n"
                           "[C++]\nfunctions=Color(0.4,0.64,0.81,1)\nstrings=Color(0.5, 0.5, 0.5,1)\ncomments=Color(0, 0.5, 0,1)\npreprocessor=Color(0.5, 0.25, 0,1)\nkeywords=Color(0.5, 0, 0.5,1)\n"
                           "[Rust]\nfunctions=Color(0.4,0.64,0.81,1)\nstrings=Color(0.5, 0.5, 0.5,1)\ncomments=Color(0, 0.5, 0,1)\nnkeywords=Color(0, 0, 0.5,1)\ntypes=Color(0, 0, 0.5,1)");
        file->close();
    }
    file->free();

    this->load_editor_settings();
}

void EditorFile::change_project_path(String path)
{
    this->project_config = path;
    if (this->instance_defined == true)
    {
       cast_to<Sidebar>(get_node(NodePath("VBoxContainer/HBoxContainer/Sidebar")))->list_directories(path);
    }
}

void EditorFile::execute_build()
{
    ProjectManager *project_manager = cast_to<ProjectManager>((WindowDialog *)get_node(NodePath("ProjectManager")));
    project_manager->build_task(0);
}

void EditorFile::execute_clean()
{
    ProjectManager *project_manager = cast_to<ProjectManager>((WindowDialog *)get_node(NodePath("ProjectManager")));
    project_manager->build_task(1);
}

void EditorFile::execute_command(String string_command)
{
    ((MenuButton *)get_node(NodePath("VBoxContainer/TopBar/Project")))->get_popup()->set_item_disabled(0, true);
    ((MenuButton *)get_node(NodePath("VBoxContainer/TopBar/Project")))->get_popup()->set_item_disabled(1, true);
    cast_to<Sidebar>(get_node(NodePath("VBoxContainer/HBoxContainer/Sidebar")))->disable_build_buttons(true);
    ((RichTextLabel *)get_node(NodePath("VBoxContainer/HBoxContainer/VBoxContainer/Control/TabContainer/Log/Panel/TextEdit")))->add_text("Executing: " + string_command + "\n");
    const char *command = (string_command + " 2>&1").utf8().get_data();
    if (OS::get_singleton()->get_name() == "windows")
    {
#define popen _popen
#define pclose _pclose
    }
    FILE *pipe = popen(command, "r");
    if (!pipe)
    {
        ((RichTextLabel *)get_node(NodePath("VBoxContainer/HBoxContainer/VBoxContainer/Control/TabContainer/Log/Panel/TextEdit")))->append_bbcode("[color=red]There was an error executing popen()[/color]");
        return;
    }
    char buffer[256];
    while (!feof(pipe))
    {
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            ((RichTextLabel *)get_node(NodePath("VBoxContainer/HBoxContainer/VBoxContainer/Control/TabContainer/Log/Panel/TextEdit")))->add_text(buffer);
        }
    }
    int status = pclose(pipe);
    String text = "[color=blue]Process returned: " + String::num_int64(status) + "[/color]\n";
    if (status != 0)
    {
        text = "[color=red]Process returned: " + String::num_int64(status) + "[/color]\n";
    }
    ((RichTextLabel *)get_node(NodePath("VBoxContainer/HBoxContainer/VBoxContainer/Control/TabContainer/Log/Panel/TextEdit")))->append_bbcode(text);
    
    ((MenuButton *)get_node(NodePath("VBoxContainer/TopBar/Project")))->get_popup()->set_item_disabled(0, false);
    ((MenuButton *)get_node(NodePath("VBoxContainer/TopBar/Project")))->get_popup()->set_item_disabled(1, false);
    cast_to<Sidebar>(get_node(NodePath("VBoxContainer/HBoxContainer/Sidebar")))->disable_build_buttons(false);
}

void EditorFile::on_file_pressed(int index)
{
    switch (index)
    {
    case 0:
        ((WindowDialog *)get_node(NodePath("ProjectManager")))->popup_centered();
        break;
    case 1:
        ((FileDialog *)get_node(NodePath("NewFile")))->popup_centered();
        break;
    case 2:
        ((FileDialog *)get_node(NodePath("OpenFile")))->popup_centered();
        break;
    case 3:
        this->_on_TabContainer_tab_close(((Tabs *)get_node("VBoxContainer/HBoxContainer/VBoxContainer/TabContainer"))->get_current_tab());
        break;
    case 4:
        save_file();
        break;
    case 5:
        Godot::print("Save as");
        break;
    }
}

void EditorFile::on_project_pressed(int index)
{
    switch (index)
    {
    case 0:
        this->execute_build();
        break;
    }
}

void EditorFile::on_settings_pressed(int index)
{
    switch (index)
    {
    case 0:
        ((WindowDialog *)get_node("Settings"))->popup_centered();
        break;
    case 1:
        cast_to<ProjectSettings>((WindowDialog *)get_node("ProjectSettings"))->setup();
        ((WindowDialog *)get_node("ProjectSettings"))->popup_centered();
        break;
    }
}

void EditorFile::_on_NewFile_file_selected(String path)
{
    File *file = File::_new();
    file->open(path, File::WRITE);
    file->close();
    file->free();
    open_file(path);
    if (this->project_config != "")
    {
        cast_to<Sidebar>(get_node(NodePath("VBoxContainer/HBoxContainer/Sidebar")))->list_directories(this->project_config);
    }
}

void EditorFile::_on_OpenFile_file_selected(String path)
{
    int file_extension_size = path.split(".").size();
    String file_extension = path.split(".")[file_extension_size - 1];
    if (file_extension == "gdnproj")
    {
        this->project_config = path;
    }
    open_file(path);
}

void EditorFile::_on_TabContainer_tab_changed(int tab)
{
    if (this->instance_defined == true)
    {
        this->current_editor_instance->hide();
    }
    this->current_editor_instance = cast_to<CodeEditor>(((Panel *)get_node("VBoxContainer/HBoxContainer/VBoxContainer/Editor"))->get_child(tab));
    this->current_editor_instance->show();
    this->file_name = ((Tabs *)get_node("VBoxContainer/HBoxContainer/VBoxContainer/TabContainer"))->get_tab_title(tab);
}

void EditorFile::_on_TabContainer_tab_close(int tab)
{
    this->instance_defined = false;
    this->file_name = "";
    this->file_path = "";
    ((Panel *)get_node("VBoxContainer/HBoxContainer/VBoxContainer/Editor"))->get_child(tab)->queue_free();
    this->tab_number = ((Panel *)get_node("VBoxContainer/HBoxContainer/VBoxContainer/Editor"))->get_child_count();
    if (this->tab_number > 1)
    {
        if (tab == 0)
        {
            this->_on_TabContainer_tab_changed(tab + 1);
        }
        else
        {
            this->_on_TabContainer_tab_changed(tab - 1);
        }
    }
    ((Tabs *)get_node("VBoxContainer/HBoxContainer/VBoxContainer/TabContainer"))->remove_tab(tab);
}

void EditorFile::_process()
{
    if (this->instance_defined == true)
    {
        if (this->current_editor_instance->get_text_changed() == true)
        {
            this->tabNode->set_tab_title(this->tabNode->get_current_tab(), this->file_name + "(*)");
        }
        else
        {
            this->tabNode->set_tab_title(this->tabNode->get_current_tab(), this->file_name);
        }
    }
}

void EditorFile::_register_methods()
{
    register_method((char *)"_init", &EditorFile::_init);
    register_method((char *)"_ready", &EditorFile::_ready);
    register_method((char *)"on_file_pressed", &EditorFile::on_file_pressed);
    register_method((char *)"on_project_pressed", &EditorFile::on_project_pressed);
    register_method((char *)"on_settings_pressed", &EditorFile::on_settings_pressed);
    register_method((char *)"open_file", &EditorFile::open_file);
    register_method((char *)"save_file", &EditorFile::save_file);
    register_method((char *)"load_config", &EditorFile::load_config);
    register_method((char *)"load_editor_settings", &EditorFile::load_editor_settings);
    register_method((char *)"load_color_settings", &EditorFile::load_color_settings);
    register_method((char *)"create_shortcuts", &EditorFile::create_shortcuts);
    register_method((char *)"change_project_path", &EditorFile::change_project_path);
    register_method((char *)"execute_build", &EditorFile::execute_build);
    register_method((char *)"execute_clean", &EditorFile::execute_clean);
    register_method((char *)"execute_command", &EditorFile::execute_command);
    register_method((char *)"create_user_data", &EditorFile::create_user_data);
    register_method((char *)"get_project_path", &EditorFile::get_project_path);
    register_method((char *)"get_selected_platform", &EditorFile::get_selected_platform);
    register_method((char *)"get_selected_profile", &EditorFile::get_selected_profile);
    register_method((char *)"get_editor_instance", &EditorFile::get_editor_instance);
    register_method((char *)"get_project_lang", &EditorFile::get_project_lang);
    register_method((char *)"get_file_icon", &EditorFile::get_file_icon);

    register_method((char *)"_on_NewFile_file_selected", &EditorFile::_on_NewFile_file_selected);
    register_method((char *)"_on_OpenFile_file_selected", &EditorFile::_on_OpenFile_file_selected);
    register_method((char *)"_on_TabContainer_tab_changed", &EditorFile::_on_TabContainer_tab_changed);
    register_method((char *)"_on_TabContainer_tab_close", &EditorFile::_on_TabContainer_tab_close);
    register_method((char *)"_process", &EditorFile::_process);
}

#include <Godot.hpp>
#include <Control.hpp>
#include <File.hpp>
#include <FileDialog.hpp>
#include <WindowDialog.hpp>
#include <MenuButton.hpp>
#include <PopupMenu.hpp>
#include <InputEventKey.hpp>
#include <Texture.hpp>
#include <ConfigFile.hpp>
#include <Theme.hpp>

#include "FileManager.hpp"
#include "ProjectManager.hpp"
#include "Settings.hpp"
using namespace godot;

EditorFile::EditorFile()
{
}

EditorFile::~EditorFile()
{
}

void EditorFile::_init()
{
    this->settings_scene = ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Scenes/Settings.tscn");
    this->code_scene = ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Scenes/CodeEditor.tscn");
}

void EditorFile::_ready()
{
    this->tabNode = ((Tabs *)get_node("TabContainer"));
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->connect("id_pressed", this, "on_file_pressed");
    ((MenuButton *)get_node(NodePath("TopBar/Project")))->get_popup()->connect("id_pressed", this, "on_project_pressed");
    ((MenuButton *)get_node(NodePath("TopBar/Settings")))->get_popup()->connect("id_pressed", this, "on_settings_pressed");
    create_shortcuts();
    create_user_data();
}

CodeEditor *EditorFile::get_editor_instance()
{
    return this->current_editor_instance;
}

void EditorFile::change_project_path(String path)
{
    this->project_config = path;
}

void EditorFile::execute_build()
{
    ProjectManager *project_manager = cast_to<ProjectManager>((WindowDialog *)get_node(NodePath("ProjectManager")));
    project_manager->build_task();
}

void EditorFile::create_user_data()
{
    File *file = File::_new();
    if (file->file_exists("user://editor.cfg") == false)
    {
        file->open("user://editor.cfg", File::WRITE);
        file->store_string("[Editor]\ncustom_font=\"res://addons/cpp_file_editor/Fonts/RobotoSlab-VariableFont_wght.ttf\"\nfont_size=12\ncustom_theme=\"res://addons/cpp_file_editor/Themes/godot_theme.tres\"");
        file->close();
    }
    file->free();

    this->load_editor_settings();
}

void EditorFile::load_editor_settings()
{
    ConfigFile *config_file = ConfigFile::_new();
    config_file->load("user://editor.cfg");

    this->custom_font = config_file->get_value("Editor", "custom_font");
    this->font_size = config_file->get_value("Editor", "font_size");
    this->custom_theme = config_file->get_value("Editor", "custom_theme");

    if (this->tab_number > 0)
    {
        for (int i = 0; i < this->tab_number; i++)
        {
            cast_to<CodeEditor>(((Tabs *)get_node("TabContainer"))->get_children()[i])->set_custom_font(this->custom_font);
            cast_to<CodeEditor>(((Tabs *)get_node("TabContainer"))->get_children()[i])->set_font_size(this->font_size);
        }
    }

    Ref<Theme> theme = ResourceLoader::get_singleton()->load(this->custom_theme);
    cast_to<Control>(this)->set_theme(theme);
    config_file->free();
}

String EditorFile::get_project_path()
{
    return this->project_config;
}

String EditorFile::get_selected_platform()
{
    return this->current_editor_instance->get_build_platform_cpp();
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
        this->_on_TabContainer_tab_close(((Tabs *)get_node("TabContainer"))->get_current_tab());
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
        ;
        break;
    }
}

void EditorFile::open_file(String path)
{
    File *file = File::_new();
    file->open(path, File::READ);
    this->current_editor_instance = cast_to<CodeEditor>(this->code_scene->instance());
    String content = file->get_as_text();
    file->close();
    file->free();
    ((Tabs *)get_node("TabContainer"))->add_child(this->current_editor_instance, true);
    this->current_editor_instance->set_initial_content(content);
    this->current_editor_instance->set_custom_font(this->custom_font);
    this->current_editor_instance->set_font_size(this->font_size);
    this->current_editor_instance->set_custom_theme(this->custom_theme);
    this->tab_number = ((Tabs *)get_node("TabContainer"))->get_child_count();
    this->file_path = path;

    int name_split_size = path.split("/").size();
    this->file_name = path.split("/")[name_split_size - 1];

    int path_split_size = this->file_name.split(".").size();
    String file_extension = this->file_name.split(".")[path_split_size - 1];

    Ref<Texture> icon;

    if (file_extension == "cpp")
    {
        this->current_editor_instance->setup_language("cpp");
        icon = ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Icons/cplusplus-original.svg", "Texture");
    }
    else if (file_extension == "rs")
    {
        this->current_editor_instance->setup_language("rust");
        icon = ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Icons/rust-plain.svg", "Texture");
    }
    else
    {
        icon = ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Icons/file.svg", "Texture");
    }

    ((Tabs *)get_node("TabContainer"))->add_tab(this->file_name, icon);
    ((Tabs *)get_node("TabContainer"))->set_current_tab(this->tab_number - 1);
    this->instance_defined = true;
}

void EditorFile::save_file()
{
    File *file = File::_new();
    file->open(file_path, File::WRITE);
    file->store_string(this->current_editor_instance->get_content());
    this->current_editor_instance->save_contents();
    file->close();
    file->free();
}

void EditorFile::_on_NewFile_file_selected(String path)
{
    File *file = File::_new();
    file->open(path, File::WRITE);
    file->close();
    file->free();
    open_file(path);
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
    this->current_editor_instance = cast_to<CodeEditor>(((Tabs *)get_node("TabContainer"))->get_child(tab));
    this->current_editor_instance->show();
    this->file_name = ((Tabs *)get_node("TabContainer"))->get_tab_title(tab);
}

void EditorFile::_on_TabContainer_tab_close(int tab)
{
    this->instance_defined = false;
    this->file_name = "";
    this->file_path = "";
    ((Tabs *)get_node("TabContainer"))->get_child(tab)->queue_free();
    this->tab_number = ((Tabs *)get_node("TabContainer"))->get_child_count();
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
    ((Tabs *)get_node("TabContainer"))->remove_tab(tab);
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

void EditorFile::create_shortcuts()
{
    Ref<InputEventKey> hotkey;

    hotkey.instance();
    hotkey->set_scancode(80);
    hotkey->set_control(true);
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->set_item_accelerator(0, hotkey->get_scancode_with_modifiers());

    hotkey->set_scancode(78);
    hotkey->set_control(true);
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->set_item_accelerator(1, hotkey->get_scancode_with_modifiers());

    hotkey->set_scancode(79);
    hotkey->set_control(true);
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->set_item_accelerator(2, hotkey->get_scancode_with_modifiers());

    hotkey->set_scancode(52);
    hotkey->set_control(true);
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->set_item_accelerator(3, hotkey->get_scancode_with_modifiers());

    hotkey->set_scancode(83);
    hotkey->set_control(true);
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->set_item_accelerator(5, hotkey->get_scancode_with_modifiers());

    hotkey->set_scancode(83);
    hotkey->set_control(true);
    hotkey->set_alt(true);
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->set_item_accelerator(6, hotkey->get_scancode_with_modifiers());
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
    register_method((char *)"create_shortcuts", &EditorFile::create_shortcuts);
    register_method((char *)"change_project_path", &EditorFile::change_project_path);
    register_method((char *)"execute_build", &EditorFile::execute_build);
    register_method((char *)"create_user_data", &EditorFile::create_user_data);
    register_method((char *)"load_editor_settings", &EditorFile::load_editor_settings);
    register_method((char *)"get_project_path", &EditorFile::get_project_path);
    register_method((char *)"get_selected_platform", &EditorFile::get_selected_platform);
    register_method((char *)"get_editor_instance", &EditorFile::get_editor_instance);

    register_method((char *)"_on_NewFile_file_selected", &EditorFile::_on_NewFile_file_selected);
    register_method((char *)"_on_OpenFile_file_selected", &EditorFile::_on_OpenFile_file_selected);
    register_method((char *)"_on_TabContainer_tab_changed", &EditorFile::_on_TabContainer_tab_changed);
    register_method((char *)"_on_TabContainer_tab_close", &EditorFile::_on_TabContainer_tab_close);
    register_method((char *)"_process", &EditorFile::_process);
}

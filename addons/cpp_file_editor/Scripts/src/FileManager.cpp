#include <Godot.hpp>
#include <Control.hpp>
#include <PackedScene.hpp>
#include <ResourceLoader.hpp>
#include <String.hpp>
#include <Tabs.hpp>
#include <Texture.hpp>
#include <MenuButton.hpp>
#include <File.hpp>
#include <FileDialog.hpp>
#include <WindowDialog.hpp>
#include <PopupMenu.hpp>
#include <InputEventKey.hpp>
#include <TabContainer.hpp>
#include <LineEdit.hpp>

#include "FileManager.hpp"
using namespace godot;

EditorFile::EditorFile()
{
}

EditorFile::~EditorFile()
{
}

void EditorFile::_init()
{
    this->code_scene = ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Scenes/CodeEditor.tscn");
}

void EditorFile::_ready()
{
    this->tabNode = ((Tabs *)get_node("TabContainer"));
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->connect("id_pressed", this, "on_file_pressed");
    create_shortcuts();
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

void EditorFile::open_file(String path)
{
    File *file = File::_new();
    file->open(path, File::READ);
    Node *new_instanced_scene = code_scene->instance();
    String content = file->get_as_text();
    file->close();
    file->free();
    ((Tabs *)get_node("TabContainer"))->add_child(new_instanced_scene, true);
    cast_to<CodeEditor>(new_instanced_scene)->set_initial_content(content);
    this->tab_number = ((Tabs *)get_node("TabContainer"))->get_child_count();
    this->file_path = path;
    this->file_name = path.split("//")[1];
    this->current_editor_instance = cast_to<CodeEditor>(((Tabs *)get_node("TabContainer"))->get_child(this->tab_number - 1));
    ((Tabs *)get_node("TabContainer"))->add_tab(this->file_name);
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

void EditorFile::_on_CancelButton_pressed()
{
    ((WindowDialog *)get_node(NodePath("ProjectManager")))->hide();
}

void EditorFile::_on_NewClassButton_pressed()
{
    ((FileDialog *)get_node(NodePath("ProjectManager/FolderPath")))->popup_centered();
}

void EditorFile::_on_FolderPath_dir_selected(String path)
{
    switch (((TabContainer *)get_node(NodePath("ProjectManager/TabContainer")))->get_current_tab())
    {
    case 0:
        ((LineEdit *)get_node(NodePath("ProjectManager/TabContainer/NewClass/PathLabel/FilePath")))->set_text(path);
        break;
    case 1:
        ((LineEdit *)get_node(NodePath("ProjectManager/TabContainer/NewProject/PathLabel/FilePath")))->set_text(path);
        break;
    }
}

void EditorFile::_on_ClassName_text_changed(String new_text)
{
    ((LineEdit *)get_node(NodePath("ProjectManager/TabContainer/NewClass/Main")))->set_text(new_text+".cpp");
    ((LineEdit *)get_node(NodePath("ProjectManager/TabContainer/NewClass/Include")))->set_text(new_text+".h");
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
    register_method((char *)"open_file", &EditorFile::open_file);
    register_method((char *)"save_file", &EditorFile::save_file);
    register_method((char *)"_on_NewFile_file_selected", &EditorFile::_on_NewFile_file_selected);
    register_method((char *)"_on_OpenFile_file_selected", &EditorFile::_on_OpenFile_file_selected);
    register_method((char *)"_on_TabContainer_tab_changed", &EditorFile::_on_TabContainer_tab_changed);
    register_method((char *)"_on_TabContainer_tab_close", &EditorFile::_on_TabContainer_tab_close);
    register_method((char *)"_on_CancelButton_pressed", &EditorFile::_on_CancelButton_pressed);
    register_method((char *)"_on_NewClassButton_pressed", &EditorFile::_on_NewClassButton_pressed);
    register_method((char *)"_on_FolderPath_dir_selected", &EditorFile::_on_FolderPath_dir_selected);
    register_method((char *)"_on_ClassName_text_changed", &EditorFile::_on_ClassName_text_changed);
    register_method((char *)"create_shortcuts", &EditorFile::create_shortcuts);
    register_method((char *)"_process", &EditorFile::_process);
}

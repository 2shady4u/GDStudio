#include <Godot.hpp>
#include <Control.hpp>
#include <PackedScene.hpp>
#include <ResourceLoader.hpp>
#include <String.hpp>
#include <TabContainer.hpp>
#include <MenuButton.hpp>
#include <File.hpp>
#include <FileDialog.hpp>
#include <PopupMenu.hpp>
#include <Input.hpp>
#include <InputEventKey.hpp>
#include <InputEventWithModifiers.hpp>
#include <OS.hpp>
#include <SceneTree.hpp>
#include <Viewport.hpp>
#include <Object.hpp>

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
    open_file("res://godot.cpp");
    this->file_path = "res://godot.cpp";
    this->file_name = this->file_path.split("//")[1];
    this->current_editor_instance = cast_to<CodeEditor>(((TabContainer *)get_node("TabContainer"))->get_child(0));
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->connect("id_pressed", this, "on_file_pressed");
    create_shortcuts();
}

void EditorFile::on_file_pressed(int index)
{
    switch (index)
    {
    case 0:
        ((FileDialog *)get_node(NodePath("NewFile")))->popup_centered();
        break;
    case 1:
        save_file();
        break;
    case 2:
        Godot::print("somethin2");
        break;
    case 3:
        ((FileDialog *)get_node(NodePath("OpenFile")))->popup_centered();
        break;
    case 4:
        Godot::print("something4");
        break;
    }
}

void EditorFile::open_file(String path)
{
    File *file = File::_new();
    file->open(path, File::READ);
    Node* new_instanced_scene = code_scene->instance();
    String content = file->get_as_text();
    file->close();
    file->free();
    ((TabContainer *)get_node("TabContainer"))->add_child(new_instanced_scene, true);
    cast_to<CodeEditor>(new_instanced_scene)->set_initial_content(content);
    this->tab_number = ((TabContainer *)get_node("TabContainer"))->get_child_count();
    String name = path.split("//")[1];
    ((TabContainer *)get_node("TabContainer"))->set_tab_title(tab_number - 1, name);
}

void EditorFile::save_file()
{
    File *file = File::_new();
    file->open(file_path, File::WRITE);
    file->store_string(current_editor_instance->get_content());
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

void EditorFile::create_shortcuts()
{
    Ref<InputEventKey> hotkey;

    hotkey.instance();
    hotkey->set_scancode(83);
    hotkey->set_control(true);
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->set_item_accelerator(2, hotkey->get_scancode_with_modifiers());

    hotkey->set_scancode(78);
    hotkey->set_control(true);
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->set_item_accelerator(0, hotkey->get_scancode_with_modifiers());

    hotkey->set_scancode(82);
    hotkey->set_control(true);
    hotkey->set_alt(true);
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->set_item_accelerator(3, hotkey->get_scancode_with_modifiers());

    hotkey->set_scancode(79);
    hotkey->set_control(true);
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->set_item_accelerator(5, hotkey->get_scancode_with_modifiers());

}

void EditorFile::_on_TabContainer_tab_changed(int tab)
{
    this->file_path = ((TabContainer *)get_node("TabContainer"))->get_tab_title(tab);
    this->file_name = this->file_path;
    this->current_editor_instance = cast_to<CodeEditor>(((TabContainer *)get_node("TabContainer"))->get_child(tab));
}

void EditorFile::_process()
{
    TabContainer *tabNode = ((TabContainer *)get_node("TabContainer"));
    if (this->current_editor_instance->get_text_changed() == true) {
        tabNode->set_tab_title(tabNode->get_current_tab(), file_name + "(*)");
    }else{
        tabNode->set_tab_title(tabNode->get_current_tab(), file_name);
    }
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
    register_method((char *)"create_shortcuts", &EditorFile::create_shortcuts);
    register_method((char *)"_on_TabContainer_tab_changed", &EditorFile::_on_TabContainer_tab_changed);
    register_method((char *)"_process", &EditorFile::_process);
}

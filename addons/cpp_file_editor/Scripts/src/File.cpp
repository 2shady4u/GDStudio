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

#include "File.hpp"
using namespace godot;

EditorFile::EditorFile() {

}
EditorFile::~EditorFile() {
    
}

void EditorFile::_init() {
    code_scene = ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Scenes/CodeEditor.tscn");
}

void EditorFile::_ready() {
    open_file("res://godot.cpp");
    Godot::print("iu");
	file_path = "res://godot.cpp";
	file_name = file_path.split("//")[1];
    current_editor_instance = ((TabContainer *)get_node("TabContainer"))->get_child(0);
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->connect("id_pressed",this,"on_file_pressed");
    create_shortcuts();
}

void EditorFile::on_file_pressed(int index) {
    switch (index) {
        case 0:
            ((FileDialog *)get_node(NodePath("NewFile")))->popup_centered();
            break;
        case 1:
            this->save_file();
            break;
        case 3:
            ((FileDialog *)get_node(NodePath("OpenFile")))->popup_centered();
    }
}

void EditorFile::open_file(String path) {
    File *file = File::_new();
    file->open(path, 1);
    Node *new_instanced_scene = code_scene->instance();
    String content = file->get_as_text();
    file->close();
    ((TabContainer *)get_node("TabContainer"))->add_child(new_instanced_scene, true);
    //new_instanced_scene->set_initial_content(content);
    tab_number = ((TabContainer *)get_node("TabContainer"))->get_child_count();
    String name = path.split("//")[1];
    ((TabContainer *)get_node("TabContainer"))->set_tab_title(tab_number - 1, name);
}

void EditorFile::create_shortcuts() {

}

void EditorFile::save_file() {

}

void EditorFile::_register_methods() {
	register_method((char *)"_init", &EditorFile::_init);
	register_method((char *)"_ready", &EditorFile::_ready);
	register_method((char *)"on_file_pressed", &EditorFile::on_file_pressed);
	register_method((char *)"open_file", &EditorFile::open_file);
	register_method((char *)"create_shortcuts", &EditorFile::create_shortcuts);

}

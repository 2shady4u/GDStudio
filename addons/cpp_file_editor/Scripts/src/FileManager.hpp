#include <Godot.hpp>
#include <Control.hpp>
#include <PackedScene.hpp>
#include <ResourceLoader.hpp>
#include <String.hpp>
#include <Tabs.hpp>
#include <thread>

#include "CodeEditor.hpp"
using namespace godot;

class EditorFile : public Control
{
    GODOT_CLASS(EditorFile, Control);

public:
    EditorFile();
    ~EditorFile();

    void _init();
    void _ready();
    void _process();
    void open_file(String);
    void on_file_pressed(int);
    void on_project_pressed(int);
    void save_file();
    void create_shortcuts();
    void create_new_class();
    void create_new_project();
    void check_thread();
    void create_rust_project(String);

    void _on_NewFile_file_selected(String);
    void _on_OpenFile_file_selected(String);
    void _on_TabContainer_tab_changed(int);
    void _on_TabContainer_tab_close(int);
    void _on_CancelButton_pressed();
    void _on_NewClassButton_pressed();
    void _on_FolderPath_dir_selected(String);
    void _on_ClassName_text_changed(String);
    void _on_OkButton_pressed();
    void _on_ProjectType_item_selected(int);
    void _on_PathButton_pressed();
    void _on_SearchCPPButton_pressed();
    void _on_cppPathSearch_dir_selected(String);

    static void _register_methods();

private:
    String file_name = "";
    String file_path = "";
    String project_config = "";
    Ref<PackedScene> code_scene;
    CodeEditor *current_editor_instance;
    int tab_number = 0;
    bool instance_defined = false;
    Tabs *tabNode;
    std::thread *thread = nullptr;
};
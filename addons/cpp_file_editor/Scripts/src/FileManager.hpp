#include <Godot.hpp>
#include <Control.hpp>
#include <PackedScene.hpp>
#include <ResourceLoader.hpp>
#include <String.hpp>
#include <Tabs.hpp>

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
    void on_settings_pressed(int);
    void save_file();
    void create_shortcuts();
    void change_project_path(String);
    void execute_build();
    void create_user_data();
    void load_editor_settings();
    String get_project_path();
    String get_selected_platform();
    bool get_selected_profile();
    CodeEditor *get_editor_instance();

    void _on_NewFile_file_selected(String);
    void _on_OpenFile_file_selected(String);
    void _on_TabContainer_tab_changed(int);
    void _on_TabContainer_tab_close(int);

    static void _register_methods();

private:
    String file_name = "";
    String file_path = "";
    String project_config = "";
    Ref<PackedScene> code_scene;
    Ref<PackedScene> settings_scene;
    CodeEditor *current_editor_instance;
    int tab_number = 0;
    bool instance_defined = false;
    Tabs *tabNode;
    String custom_font = "";
    int font_size = 0;
    String custom_theme = "";
};
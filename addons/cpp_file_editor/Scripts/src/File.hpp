#include <Godot.hpp>
#include <Control.hpp>
#include <PackedScene.hpp>
#include <ResourceLoader.hpp>
#include <String.hpp>

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
    void create_shortcuts();
    void on_file_pressed(int);
    void save_file();
    void _on_NewFile_file_selected(String);
    void _on_OpenFile_file_selected(String);
    void _on_TabContainer_tab_changed(int);

	static void _register_methods();
    
private:
    String file_name = "";
    String file_path = "";
    Ref<PackedScene> code_scene;
    Node *current_editor_instance;
    int tab_number = 0;
};
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
    void open_file(String);
    void create_shortcuts();
    void on_file_pressed(int);
    void save_file();

	static void _register_methods();
    
private:
    String file_name = "";
    String file_path = "";
    Ref<PackedScene> code_scene;
    Node *current_editor_instance;
    int tab_number = 0;
};
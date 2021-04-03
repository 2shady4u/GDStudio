#include <Godot.hpp>
#include <Control.hpp>
#include <Tree.hpp>

using namespace godot;

class Settings : public Control
{
    GODOT_CLASS(Settings, Control);

public:
    Settings();
    ~Settings();
    void _init();
    void _ready();
    void show_window();
    void set_editor_data();
    void save_editor_data();

    void _on_ConfirmSettings_pressed();
    void _on_EditorTree_button_pressed(TreeItem*, int, int);
    void _on_OpenFile_file_selected(String);

    static void _register_methods();

private:
    Tree *tree;
    int selected_id = -1;

};
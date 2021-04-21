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
    void set_initial_values();
    void set_editor_data();
    void set_cpp_data();
    void set_rust_data();
    void save_editor_data();
    void save_cpp_data();
    void save_rust_data();

    void _on_ConfirmSettings_pressed();
    void _on_ResetSettings_pressed();
    void _on_SearchFont_pressed();
    void _on_SearchTheme_pressed();
    void _on_OpenFile_file_selected(String);
    void _on_CategoryTree_item_selected();

    static void _register_methods();

private:
    Tree *tree;
    int selected_id = -1;

};
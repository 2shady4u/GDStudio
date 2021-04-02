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

    static void _register_methods();

private:
    Tree *tree;

};
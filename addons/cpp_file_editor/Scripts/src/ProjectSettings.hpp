#include <Godot.hpp>
#include <Control.hpp>

using namespace godot;

#ifndef ProjectSettings_HEADER
#define ProjectSettings_HEADER

class ProjectSettings : public Control
{
    GODOT_CLASS(ProjectSettings, Control);

public:
    ProjectSettings();
    ~ProjectSettings();
    void _init();
    void _ready();
    void setup();
    void load_settings(String);

    void _on_ConfirmSettings_pressed();
    void _on_AddIncludePath_pressed();
    void _on_EditIncludePath_pressed();
    void _on_RemoveIncludePath_pressed();
    void _on_ClearIncludePath_pressed();
    void _on_OpenFolder_dir_selected(String);
    
    static void _register_methods();

private:
    String project_lang;
    int folder_return = -1;
};

#endif
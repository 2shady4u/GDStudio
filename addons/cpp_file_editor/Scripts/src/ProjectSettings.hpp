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

    static void _register_methods();

private:
    String project_lang;
};

#endif
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
};

#endif
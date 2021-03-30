#include <Godot.hpp>
#include <Control.hpp>

using namespace godot;

#ifndef ProjectManager_HEADER
#define ProjectManager_HEADER
class ProjectManager : public Control
{
    GODOT_CLASS(ProjectManager, Control);

public:
    ProjectManager();
    ~ProjectManager();

private:
};
#endif
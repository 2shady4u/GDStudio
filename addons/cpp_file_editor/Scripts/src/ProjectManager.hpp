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
    void build_cpp_project(String,String);
    void build_rust_project(String,String);

    static void register_methods();

private:
};
#endif
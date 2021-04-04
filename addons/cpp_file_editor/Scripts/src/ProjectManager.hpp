#include <Godot.hpp>
#include <Control.hpp>
#include <thread>

using namespace godot;

#ifndef ProjectManager_HEADER
#define ProjectManager_HEADER
class ProjectManager : public Control
{
    GODOT_CLASS(ProjectManager, Control);

public:
    ProjectManager();
    ~ProjectManager();
    void _init();
    void _ready();
    void build_task();
    void build_cpp_project(String,String);
    void build_rust_project(String,String);
    void create_new_class();
    void create_new_project();
    void create_rust_project(String);
    void check_thread();
    void execute_os(String, PoolStringArray, bool);

    void _on_OkButton_pressed();
    void _on_CancelButton_pressed();
    void _on_FolderPath_dir_selected(String);
    void _on_NewClassButton_pressed();
    void _on_ClassName_text_changed(String);
    void _on_ProjectType_item_selected(int);
    void _on_PathButton_pressed();
    void _on_SearchCPPButton_pressed();
    void _on_cppPathSearch_dir_selected(String);

    static void _register_methods();

private:
    std::thread *thread = nullptr;
};
#endif
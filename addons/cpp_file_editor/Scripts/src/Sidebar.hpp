#include <Godot.hpp>
#include <Control.hpp>
#include <Tree.hpp>
#include <TreeItem.hpp>
#include <MainLoop.hpp>

using namespace godot;

#ifndef Sidebar_HEADER
#define Sidebar_HEADER
class Sidebar : public Control
{
    GODOT_CLASS(Sidebar, Control);

public:
    Sidebar();
    ~Sidebar();
    void _init();
    void _notification(int);
    bool get_release_flag();
    void list_directories(String);
    void list_subdirectories(String, TreeItem *);
    String get_build_platform();

    void _on_Build_pressed();
    void _on_Clean_pressed();
    void _on_ExecuteCustomCommandButton_pressed();

    static void _register_methods();
};

#endif

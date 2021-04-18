#include <Godot.hpp>
#include <Control.hpp>

using namespace godot;

class BottomPanel : public Control
{
    GODOT_CLASS(BottomPanel, Control);

public:
    BottomPanel();
    ~BottomPanel();
    void _init();
    void _ready();

    void _on_TabContainer_tab_selected(int);
    
    static void _register_methods();

private:
    int tab_selected;
    bool visible = true;
};

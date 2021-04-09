#include <Godot.hpp>
#include <Control.hpp>

#include "BottomPanel.hpp"
using namespace godot;

BottomPanel::BottomPanel()
{
}
BottomPanel::~BottomPanel()
{
}

void BottomPanel::_init()
{
    this->tab_selected = 0;
}

void BottomPanel::_ready()
{
}

void BottomPanel::_on_TabContainer_tab_selected(int id)
{
    if (id == this->tab_selected)
    {
        if (this->visible == true)
        {
            ((Control *)get_node(NodePath("TabContainer/Log")))->hide();
            int x = ((Control *)get_node(NodePath("TabContainer")))->get_size()[0];
            ((Control *)get_node(NodePath("TabContainer")))->set_size(Vector2(x, 36));
            x = ((Control *)get_node(NodePath("TabContainer")))->get_position()[0];
            int y = ((Control *)get_node(NodePath("TabContainer")))->get_position()[1];
            ((Control *)get_node(NodePath("TabContainer")))->set_position(Vector2(x, y+128));
            this->visible = false;
        }
        else
        {
            ((Control *)get_node(NodePath("TabContainer/Log")))->show();
            int x = ((Control *)get_node(NodePath("TabContainer")))->get_size()[0];
            ((Control *)get_node(NodePath("TabContainer")))->set_size(Vector2(x, 164));
            x = ((Control *)get_node(NodePath("TabContainer")))->get_position()[0];
            int y = ((Control *)get_node(NodePath("TabContainer")))->get_position()[1];
            ((Control *)get_node(NodePath("TabContainer")))->set_position(Vector2(x, y-128));
            this->visible = true;
        }
    }
}

void BottomPanel::_register_methods()
{
    register_method((char *)"_init", &BottomPanel::_init);
    register_method((char *)"_ready", &BottomPanel::_ready);
    register_method((char *)"_on_TabContainer_tab_selected", &BottomPanel::_on_TabContainer_tab_selected);
}
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
            int x = this->get_size()[0];
            this->set_size(Vector2(x, 36));
            this->set_custom_minimum_size(Vector2(0, 36));
            x = this->get_position()[0];
            int y = this->get_position()[1];
            this->set_position(Vector2(x, y+128));
            this->visible = false;
        }
        else
        {
            ((Control *)get_node(NodePath("TabContainer/Log")))->show();
            int x = this->get_size()[0];
            this->set_size(Vector2(x, 164));
            this->set_custom_minimum_size(Vector2(x, 164));
            x = this->get_position()[0];
            int y = this->get_position()[1];
            this->set_position(Vector2(x, y-128));
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
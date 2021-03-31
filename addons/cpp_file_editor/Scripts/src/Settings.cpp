#include <Godot.hpp>
#include <Control.hpp>
#include <TreeItem.hpp>
#include <WindowDialog.hpp>

#include "Settings.hpp"
using namespace godot;

Settings::Settings()
{
}

Settings::~Settings()
{
}

void Settings::_init()
{
}

void Settings::_ready()
{
    tree = ((Tree *)get_node(NodePath("VBoxContainer/Settings/Container/SettingsTree")));
    TreeItem *general = tree->create_item();
    general->set_text(0, "General");
}

void Settings::show_window()
{
    cast_to<WindowDialog>(this)->popup_centered();
}

void Settings::_register_methods()
{
    register_method((char *)"_init", &Settings::_init);
    register_method((char *)"_ready", &Settings::_ready);
    register_method((char *)"show", &Settings::show_window);
}
#include <Godot.hpp>
#include <Control.hpp>
#include <TreeItem.hpp>
#include <WindowDialog.hpp>
#include <Texture.hpp>
#include <Reference.hpp>
#include <ResourceLoader.hpp>

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
    tree = ((Tree *)get_node(NodePath("VBoxContainer/Settings/CategoryContainer/CategoryTree")));
    TreeItem *editor = tree->create_item();
    editor->set_text(0, "Editor");

    tree = ((Tree *)get_node(NodePath("VBoxContainer/Settings/SettingsContainer/EditorTree")));

    TreeItem *root = tree->create_item();
    TreeItem *font = tree->create_item(root);
    font->set_text(0, "Custom Font");
    font->set_text(1, "");
    font->set_editable(1, true);
    font->set_custom_bg_color(1, Color(1, 1, 1, 0.5));
    Ref<Texture> icon;
    icon = ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Icons/default_folder.svg", "Texture");
    font->add_button(1, icon, 0);

    TreeItem *font_size = tree->create_item(root);
    font_size->set_text(0, "Font Size");
    font_size->set_text(1, "16");
    font_size->set_editable(1, true);
    font_size->set_cell_mode(1, 2);
    font_size->set_custom_color(1, Color(1, 1, 1, 0.5));
    font_size->set_range_config(1, 6, 72, 2);

    TreeItem *theme = tree->create_item(root);
    theme->set_text(0, "Custom Theme");
    theme->set_text(1, "");
    theme->set_editable(1, true);
    theme->set_custom_bg_color(1, Color(1, 1, 1, 0.5));
    theme->add_button(1, icon, 1);

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
#include <LineEdit.hpp>
#include <OptionButton.hpp>
#include <Directory.hpp>
#include <Texture.hpp>
#include <thread>
#include <future>

#include "Sidebar.hpp"
#include "FileManager.hpp"
using namespace godot;

Sidebar::Sidebar()
{
}

Sidebar::~Sidebar()
{
}

void Sidebar::_init()
{
}

bool Sidebar::get_release_flag()
{
    int button = ((OptionButton *)get_node(NodePath("Build/Target/OptionButton")))->get_selected();
    if (button == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Sidebar::list_directories(String path)
{
    ((LineEdit *)get_node(NodePath("Explorer/VBoxContainer/Root")))->set_text(path);

    Tree *tree = ((Tree *)get_node(NodePath("Explorer/VBoxContainer/Tree")));

    TreeItem *root = tree->create_item();
    TreeItem *item;

    PoolStringArray dirs;
    PoolStringArray files;
    Directory *dir = Directory::_new();
    dir->open(path);
    dir->list_dir_begin();
    String name = dir->get_next();
    String dot = ".";
    while (name != "")
    {
        if (name.begins_with(dot) == false)
        {
            if (dir->current_is_dir() == true)
            {
                dirs.append(name);
            }
            else
            {
                files.append(name);
            }
        }

        name = dir->get_next();
    }

    for (int i = 0; i < dirs.size(); i++)
    {
        item = tree->create_item(root);
        item->set_text(0, dirs[i]);
        list_subdirectories(path + "/" + dirs[i], item);
        item->set_icon(0, cast_to<EditorFile>(this->get_parent()->get_parent()->get_parent())->get_file_icon("folder"));
    }

    for (int i = 0; i < files.size(); i++)
    {
        item = tree->create_item(root);
        item->set_text(0, files[i]);
        item->set_icon(0, cast_to<EditorFile>(this->get_parent()->get_parent()->get_parent())->get_file_icon(files[i].get_extension()));
    }
    dir->free();
}

void Sidebar::list_subdirectories(String path, TreeItem *root)
{
    Tree *tree = ((Tree *)get_node(NodePath("Explorer/VBoxContainer/Tree")));

    TreeItem *item;

    PoolStringArray dirs;
    PoolStringArray files;
    Directory *dir = Directory::_new();
    dir->open(path);
    dir->list_dir_begin();
    String name = dir->get_next();
    String dot = ".";
    while (name != "")
    {
        if (name.begins_with(dot) == false)
        {
            if (dir->current_is_dir() == true)
            {
                dirs.append(name);
            }
            else
            {
                files.append(name);
            }
        }

        name = dir->get_next();
    }

    for (int i = 0; i < dirs.size(); i++)
    {
        item = tree->create_item(root);
        item->set_text(0, dirs[i]);
        list_subdirectories(path + "/" + dirs[i], item);
        item->set_icon(0, cast_to<EditorFile>(this->get_parent()->get_parent()->get_parent())->get_file_icon("folder"));
    }

    for (int i = 0; i < files.size(); i++)
    {
        item = tree->create_item(root);
        item->set_text(0, files[i]);
        item->set_icon(0, cast_to<EditorFile>(this->get_parent()->get_parent()->get_parent())->get_file_icon(files[i].get_extension()));
    }
    dir->free();
}

String Sidebar::get_build_platform()
{
    int index = ((OptionButton *)get_node(NodePath("Build/Platform/Platform")))->get_selected_id();

    switch (index)
    {
    case 0:
        return "windows";
        break;
    case 1:
        return "x11";
        break;
    case 2:
        return "osx";
        break;
    case 3:
        return "android";
        break;
    case 4:
        return "ios";
        break;
    default:
        return "unknown";
        break;
    }
}

void Sidebar::_on_Build_pressed()
{
    EditorFile *editor = cast_to<EditorFile>(this->get_parent()->get_parent()->get_parent());
    editor->execute_build();
}

void Sidebar::_on_Clean_pressed()
{
    EditorFile *editor = cast_to<EditorFile>(this->get_parent()->get_parent()->get_parent());
    editor->execute_clean();
}

void Sidebar::_on_ExecuteCustomCommandButton_pressed()
{
    String args;
    String command = ((LineEdit *)get_node(NodePath("Build/Command/Command")))->get_text();

    std::future<void> th = std::async(std::launch::async, &EditorFile::execute_command, cast_to<EditorFile>(this->get_parent()->get_parent()->get_parent()), command);
}

void Sidebar::_register_methods()
{
    register_method((char *)"_init", &Sidebar::_init);
    register_method((char *)"get_release_flag", &Sidebar::get_release_flag);
    register_method((char *)"list_directories", &Sidebar::list_directories);
    register_method((char *)"list_subdirectories", &Sidebar::list_subdirectories);
    register_method((char *)"get_build_platform", &Sidebar::get_build_platform);

    register_method((char *)"_on_Build_pressed", &Sidebar::_on_Build_pressed);
    register_method((char *)"_on_Clean_pressed", &Sidebar::_on_Clean_pressed);
    register_method((char *)"_on_ExecuteCustomCommandButton_pressed", &Sidebar::_on_ExecuteCustomCommandButton_pressed);
}

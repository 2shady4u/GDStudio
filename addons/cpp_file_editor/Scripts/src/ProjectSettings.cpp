#include <Godot.hpp>
#include <Control.hpp>
#include <ScrollContainer.hpp>
#include <ConfigFile.hpp>
#include <LineEdit.hpp>
#include <PoolArrays.hpp>
#include <OptionButton.hpp>
#include <OS.hpp>
#include <Array.hpp>
#include <ItemList.hpp>
#include <Reference.hpp>
#include <Texture.hpp>

#include "ProjectSettings.hpp"
#include "FileManager.hpp"

using namespace godot;

ProjectSettings::ProjectSettings()
{
}

ProjectSettings::~ProjectSettings()
{
}

void ProjectSettings::_init()
{

}

void ProjectSettings::_ready()
{
}

void ProjectSettings::setup()
{
    String lang = cast_to<EditorFile>(this->get_parent())->get_project_lang();
    if (lang == "c++")
    {
        this->project_lang = "c++";
        ((ScrollContainer *)get_node(NodePath("PanelContainer/VBoxContainer/CPP")))->show();
    }
    else if (lang == "rust")
    {
        this->project_lang = "rust";
        ((ScrollContainer *)get_node(NodePath("PanelContainer/VBoxContainer/Rust")))->show();
    }
    else
    {
        this->project_lang = "none";
    }

    if (lang != "none")
    {
        this->load_settings(lang);
    }
}

void ProjectSettings::load_settings(String language)
{
    ConfigFile *config_file = ConfigFile::_new();
    config_file->load(cast_to<EditorFile>(this->get_parent())->get_project_path()+"/settings.gdnproj");
    
    String project_path = config_file->get_value("settings", "path");
    ((LineEdit *)get_node(NodePath("PanelContainer/VBoxContainer/ProjectPath/LineEdit")))->set_text(project_path);
    int size = project_path.split('/').size();
    PoolStringArray path_slice = project_path.split('/');
    String project_name = path_slice[size - 1];
    ((LineEdit *)get_node(NodePath("PanelContainer/VBoxContainer/ProjectName/LineEdit")))->set_text(project_name);

    if (language == "c++")
    {
        ((OptionButton *)get_node(NodePath("PanelContainer/VBoxContainer/ProjectLang/OptionButton")))->select(0);
        String sources = config_file->get_value("settings", "sources_folder");
        ((LineEdit *)get_node(NodePath("PanelContainer/VBoxContainer/CPP/VBoxContainer/Sources/LineEdit")))->set_text(sources);
        String bindings = config_file->get_value("settings", "godot_cpp_folder");
        ((LineEdit *)get_node(NodePath("PanelContainer/VBoxContainer/CPP/VBoxContainer/Bindings/LineEdit")))->set_text(bindings);
        String include_string = config_file->get_value("settings", "include_folders");
        PoolStringArray include_array = include_string.split(',');
        for (int i = 0; i < include_array.size(); i++)
        {
            ((ItemList *)get_node(NodePath("PanelContainer/VBoxContainer/CPP/VBoxContainer/PanelsContainer/Include/VBoxContainer/ItemList")))->add_item(include_array[i]);
        }
        String linker_string = config_file->get_value("settings", "linker_folders");
        PoolStringArray linker_array = linker_string.split(',');
        for (int i = 0; i < linker_array.size(); i++)
        {
            ((ItemList *)get_node(NodePath("PanelContainer/VBoxContainer/CPP/VBoxContainer/PanelsContainer/Include/VBoxContainer/ItemList")))->add_item(linker_array[i]);
        }
    }
    else if (language == "rust")
    {
        ((OptionButton *)get_node(NodePath("PanelContainer/VBoxContainer/ProjectLang/OptionButton")))->select(1);
        PoolStringArray args;
        args.append("--version");
        Array output;
        String cargo_version = OS::get_singleton()->execute("cargo", args, true, output);
        String gdn_version = config_file->get_value("settings", "gnative_version");
        ((LineEdit *)get_node(NodePath("PanelContainer/VBoxContainer/Rust/VBoxContainer/CargoVersion/LineEdit")))->set_text(output[0]);
        ((LineEdit *)get_node(NodePath("PanelContainer/VBoxContainer/Rust/VBoxContainer/GDNativeVersion/LineEdit")))->set_text(gdn_version);
    }
    config_file->free();
}

void ProjectSettings::_register_methods()
{
    register_method((char *)"_init", &ProjectSettings::_init);
    register_method((char *)"_ready", &ProjectSettings::_ready);
    register_method((char *)"setup", &ProjectSettings::setup);
    register_method((char *)"load_settings", &ProjectSettings::load_settings);
}
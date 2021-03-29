#include <Godot.hpp>
#include <Control.hpp>
#include <PackedScene.hpp>
#include <ResourceLoader.hpp>
#include <String.hpp>
#include <Tabs.hpp>
#include <Texture.hpp>
#include <MenuButton.hpp>
#include <File.hpp>
#include <FileDialog.hpp>
#include <WindowDialog.hpp>
#include <PopupMenu.hpp>
#include <InputEventKey.hpp>
#include <TabContainer.hpp>
#include <LineEdit.hpp>
#include <AcceptDialog.hpp>
#include <MenuButton.hpp>
#include <OptionButton.hpp>
#include <Directory.hpp>
#include <OS.hpp>
#include <PoolArrays.hpp>

#include "FileManager.hpp"
using namespace godot;

EditorFile::EditorFile()
{
}

EditorFile::~EditorFile()
{
}

void EditorFile::_init()
{
    this->code_scene = ResourceLoader::get_singleton()->load("res://addons/cpp_file_editor/Scenes/CodeEditor.tscn");
}

void EditorFile::_ready()
{
    this->tabNode = ((Tabs *)get_node("TabContainer"));
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->connect("id_pressed", this, "on_file_pressed");
    create_shortcuts();
}

void EditorFile::on_file_pressed(int index)
{
    switch (index)
    {
    case 0:
        ((WindowDialog *)get_node(NodePath("ProjectManager")))->popup_centered();
        break;
    case 1:
        ((FileDialog *)get_node(NodePath("NewFile")))->popup_centered();
        break;
    case 2:
        ((FileDialog *)get_node(NodePath("OpenFile")))->popup_centered();
        break;
    case 3:
        this->_on_TabContainer_tab_close(((Tabs *)get_node("TabContainer"))->get_current_tab());
        break;
    case 4:
        save_file();
        break;
    case 5:
        Godot::print("Save as");
        break;
    }
}

void EditorFile::open_file(String path)
{
    File *file = File::_new();
    file->open(path, File::READ);
    Node *new_instanced_scene = code_scene->instance();
    String content = file->get_as_text();
    file->close();
    file->free();
    ((Tabs *)get_node("TabContainer"))->add_child(new_instanced_scene, true);
    cast_to<CodeEditor>(new_instanced_scene)->set_initial_content(content);
    this->tab_number = ((Tabs *)get_node("TabContainer"))->get_child_count();
    this->file_path = path;
    this->file_name = path.split("//")[1];
    this->current_editor_instance = cast_to<CodeEditor>(((Tabs *)get_node("TabContainer"))->get_child(this->tab_number - 1));
    ((Tabs *)get_node("TabContainer"))->add_tab(this->file_name);
    ((Tabs *)get_node("TabContainer"))->set_current_tab(this->tab_number - 1);
    this->instance_defined = true;
}

void EditorFile::save_file()
{
    File *file = File::_new();
    file->open(file_path, File::WRITE);
    file->store_string(this->current_editor_instance->get_content());
    this->current_editor_instance->save_contents();
    file->close();
    file->free();
}

void EditorFile::_on_NewFile_file_selected(String path)
{
    File *file = File::_new();
    file->open(path, File::WRITE);
    file->close();
    file->free();
    open_file(path);
}

void EditorFile::_on_OpenFile_file_selected(String path)
{
    open_file(path);
}

void EditorFile::_on_TabContainer_tab_changed(int tab)
{
    if (this->instance_defined == true)
    {
        this->current_editor_instance->hide();
    }
    this->current_editor_instance = cast_to<CodeEditor>(((Tabs *)get_node("TabContainer"))->get_child(tab));
    this->current_editor_instance->show();
    this->file_name = ((Tabs *)get_node("TabContainer"))->get_tab_title(tab);
}

void EditorFile::_on_TabContainer_tab_close(int tab)
{
    this->instance_defined = false;
    this->file_name = "";
    this->file_path = "";
    ((Tabs *)get_node("TabContainer"))->get_child(tab)->queue_free();
    this->tab_number = ((Tabs *)get_node("TabContainer"))->get_child_count();
    if (this->tab_number > 1)
    {
        if (tab == 0)
        {
            this->_on_TabContainer_tab_changed(tab + 1);
        }
        else
        {
            this->_on_TabContainer_tab_changed(tab - 1);
        }
    }
    ((Tabs *)get_node("TabContainer"))->remove_tab(tab);
}

void EditorFile::_process()
{
    if (this->instance_defined == true)
    {
        if (this->current_editor_instance->get_text_changed() == true)
        {
            this->tabNode->set_tab_title(this->tabNode->get_current_tab(), this->file_name + "(*)");
        }
        else
        {
            this->tabNode->set_tab_title(this->tabNode->get_current_tab(), this->file_name);
        }
    }
}

void EditorFile::_on_CancelButton_pressed()
{
    ((WindowDialog *)get_node(NodePath("ProjectManager")))->hide();
}

void EditorFile::_on_NewClassButton_pressed()
{
    ((FileDialog *)get_node(NodePath("ProjectManager/FolderPath")))->popup_centered();
}

void EditorFile::_on_FolderPath_dir_selected(String path)
{
    switch (((TabContainer *)get_node(NodePath("ProjectManager/TabContainer")))->get_current_tab())
    {
    case 0:
        ((LineEdit *)get_node(NodePath("ProjectManager/TabContainer/NewClass/PathLabel/FilePath")))->set_text(path);
        break;
    case 1:
        ((LineEdit *)get_node(NodePath("ProjectManager/TabContainer/NewProject/PathLabel/FilePath")))->set_text(path);
        break;
    }
}

void EditorFile::_on_ClassName_text_changed(String new_text)
{
    int class_lang = ((OptionButton *)get_node(NodePath("ProjectManager/TabContainer/NewClass/ClassType/ClassType")))->get_selected_id();
    switch (class_lang)
    {
    case 0:
        ((LineEdit *)get_node(NodePath("ProjectManager/TabContainer/NewClass/Main")))->set_text(new_text + ".cpp");
        ((LineEdit *)get_node(NodePath("ProjectManager/TabContainer/NewClass/Include")))->set_text(new_text + ".h");
        break;
    case 1:
        ((LineEdit *)get_node(NodePath("ProjectManager/TabContainer/NewClass/Main")))->set_text(new_text + ".rs");
        ((LineEdit *)get_node(NodePath("ProjectManager/TabContainer/NewClass/Include")))->set_text("");
        break;
    }
}

void EditorFile::_on_OkButton_pressed()
{
    switch (((TabContainer *)get_node(NodePath("ProjectManager/TabContainer")))->get_current_tab())
    {
    case 0:
        this->create_new_class();
        break;
    case 1:
        this->create_new_project();
        break;
    }
    ((WindowDialog *)get_node(NodePath("ProjectManager")))->hide();
}

void EditorFile::_on_ProjectType_item_selected(int index)
{
    switch (index)
    {
    case 0:
        ((Control *)get_node(NodePath("ProjectManager/TabContainer/NewProject/Rust")))->hide();
        ((Control *)get_node(NodePath("ProjectManager/TabContainer/NewProject/CPP")))->show();
        break;
    case 1:
        ((Control *)get_node(NodePath("ProjectManager/TabContainer/NewProject/CPP")))->hide();
        ((Control *)get_node(NodePath("ProjectManager/TabContainer/NewProject/Rust")))->show();
        break;
    }
}

void EditorFile::_on_PathButton_pressed()
{
    ((FileDialog *)get_node(NodePath("ProjectManager/FolderPath")))->popup_centered();
}

void EditorFile::_on_SearchCPPButton_pressed()
{
    ((FileDialog *)get_node(NodePath("ProjectManager/TabContainer/NewProject/CPP/cppPathSearch")))->popup_centered();
}

void EditorFile::_on_cppPathSearch_dir_selected(String path)
{
    ((LineEdit *)get_node(NodePath("ProjectManager/TabContainer/NewProject/CPP/cppPath/cppPath")))->set_text(path);
}

void EditorFile::create_shortcuts()
{
    Ref<InputEventKey> hotkey;

    hotkey.instance();
    hotkey->set_scancode(80);
    hotkey->set_control(true);
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->set_item_accelerator(0, hotkey->get_scancode_with_modifiers());

    hotkey->set_scancode(78);
    hotkey->set_control(true);
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->set_item_accelerator(1, hotkey->get_scancode_with_modifiers());

    hotkey->set_scancode(79);
    hotkey->set_control(true);
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->set_item_accelerator(2, hotkey->get_scancode_with_modifiers());

    hotkey->set_scancode(52);
    hotkey->set_control(true);
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->set_item_accelerator(3, hotkey->get_scancode_with_modifiers());

    hotkey->set_scancode(83);
    hotkey->set_control(true);
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->set_item_accelerator(5, hotkey->get_scancode_with_modifiers());

    hotkey->set_scancode(83);
    hotkey->set_control(true);
    hotkey->set_alt(true);
    ((MenuButton *)get_node(NodePath("TopBar/File")))->get_popup()->set_item_accelerator(6, hotkey->get_scancode_with_modifiers());
}

void EditorFile::create_new_class()
{
    String class_path = ((LineEdit *)get_node(NodePath("ProjectManager/TabContainer/NewClass/PathLabel/FilePath")))->get_text();
    String class_name = ((LineEdit *)get_node(NodePath("ProjectManager/TabContainer/NewClass/ClassName/ClassName")))->get_text();
    int inherit_item = ((OptionButton *)get_node(NodePath("ProjectManager/TabContainer/NewClass/Inherit/Inherits")))->get_selected();
    String class_inherit = ((OptionButton *)get_node(NodePath("ProjectManager/TabContainer/NewClass/Inherit/Inherits")))->get_item_text(inherit_item);
    int class_lang = ((OptionButton *)get_node(NodePath("ProjectManager/TabContainer/NewClass/ClassType/ClassType")))->get_selected_id();

    if (class_name == "" || class_path == "")
    {
        ((AcceptDialog *)get_node(NodePath("ProjectManager/TabContainer/NewClass/Warning")))->popup_centered();
    }
    else
    {
        File *file = File::_new();
        if (class_lang == 0)
        {
            file->open(class_path + "/" + class_name + ".h", File::WRITE);
            file->store_string("#include <Godot.hpp>\n#include <" + class_inherit + ".hpp>\n\nclass " + class_name + " : public " + class_inherit +
                               "{\n\tGODOT_CLASS(" + class_name + "," + class_inherit + ")" +
                               "\n\nprivate:\n\npublic:\n\tstatic void _register_methods();\n\t" + class_name + "();\n\t~" +
                               class_name + "();\n\n};");
            file->close();
            file->open(class_path + "/" + class_name + ".cpp", File::WRITE);
            file->store_string("#include <" + class_name + ".h>\n\nusing namespace godot;\n\nvoid " +
                               class_name + "::_register_methods() {\n\tregister_method(\"_process\", &" +
                               class_name + "::_process);\n}\n\n" + class_name + "::" + class_name +
                               "() {\n}\n\n" + class_name + "::~" + class_name + "() {\n}");
            file->close();
        }
        else
        {
            file->open(class_path + "/" + class_name + ".rs", File::WRITE);
            file->store_string("use gdnative::api::{" + class_inherit + "};\nuse gdnative::prelude::*;\n\n#[derive(NativeClass)]\n" + "#[inherit(" + class_inherit + ")]\npub struct " + class_name + ";\n\n#[methods]\nimpl " +
                               class_name + " {\n\tfn new(_owner: &" + class_inherit + ") -> Self {\n\t\tSelf\n\t}\n}");
            file->close();
        }

        file->free();
    }
}

void EditorFile::create_new_project()
{
    String path = ((LineEdit *)get_node(NodePath("ProjectManager/TabContainer/NewProject/PathLabel/FilePath")))->get_text();
    String cpp_path = ((LineEdit *)get_node(NodePath("ProjectManager/TabContainer/NewProject/CPP/cppPath/cppPath")))->get_text();
    String source_folder = ((LineEdit *)get_node(NodePath("ProjectManager/TabContainer/NewProject/CPP/Source/Source")))->get_text();
    String scons_platform = "";
    switch (((OptionButton *)get_node(NodePath("ProjectManager/TabContainer/NewProject/CPP/Platform/Platform")))->get_selected_id())
    {
    case 0:
        scons_platform = "windows";
        break;
    case 1:
        scons_platform = "linux";
        break;
    case 2:
        scons_platform = "osx";
        break;
    }
    switch (((OptionButton *)get_node(NodePath("ProjectManager/TabContainer/NewProject/ProjectType/ProjectType")))->get_selected_id())
    {
    case 0:
        if (path == "" || cpp_path == "")
        {
            ((AcceptDialog *)get_node(NodePath("ProjectManager/TabContainer/NewProject/CPP/Warning")))->popup_centered();
        }
        else
        {
            File *file = File::_new();
            Directory *dir = Directory::_new();
            dir->open(path);
            dir->make_dir(source_folder);
            file->open(path + "/" + source_folder + "/main.cpp", File::WRITE);
            file->store_string("#include <Godot.hpp>\n\n"
                               "extern \"C\" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {\n\t"
                               "godot::Godot::gdnative_init(o);\n}\n\n"
                               "extern \"C\" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {\n\t"
                               "godot::Godot::gdnative_terminate(o);\n}\n\n"
                               "extern \"C\" void GDN_EXPORT godot_nativescript_init(void *handle) {\n\t"
                               "godot::Godot::nativescript_init(handle);\n}");
            file->close();
            file->open(path + "/SConstruct", File::WRITE);
            file->store_string("import os\n\n"
                               "platform = ARGUMENTS.get(\"p\", \"linux\")\n"
                               "platform = ARGUMENTS.get(\"platform\", platform)\n"
                               "env = Environment()\n"
                               "if platform == \"windows\":\n\t"
                               "env = Environment(ENV=os.environ)\n\n"
                               "godot_headers_path = ARGUMENTS.get(\"headers\", os.getenv(\"GODOT_HEADERS\", \"" +
                               cpp_path + "/godot_headers\"))\n"
                                          "godot_bindings_path = ARGUMENTS.get(\"cpp_bindings\", os.getenv(\"CPP_BINDINGS\", \"" +
                               cpp_path + "\"))\n\n"
                                          "target = ARGUMENTS.get(\"target\", \"debug\")\n\n"
                                          "if ARGUMENTS.get(\"use_llvm\", \"no\") == \"yes\":\n\t"
                                          "env[\"CXX\"] = \"clang++\"\n\n"
                                          "if platform == \"osx\":\n\t"
                                          "env.Append(CCFLAGS=[\"-g\", \"-O3\", \"-std=c++14\", \"-arch\", \"x86_64\"])\n\t"
                                          "env.Append(LINKFLAGS=[\"-arch\", \"x86_64\", \"-framework\", \"Cocoa\", \"-Wl,-undefined,dynamic_lookup\"])\n"
                                          "if platform == \"linux\":\n\t"
                                          "env.Append(CCFLAGS=[\"-g\", \"-O3\", \"-std=c++14\", \"-Wno-writable-strings\"])\n\t"
                                          "env.Append(LINKFLAGS=[\"-Wl,-R,'$$ORIGIN'\"])\n"
                                          "if platform == \"windows\":\n\t"
                                          "env.Append(LINKFLAGS=[\"/WX\"])\n\t"
                                          "if target == \"debug\":\n\t\t"
                                          "env.Append(CCFLAGS=[\"-EHsc\", \"-D_DEBUG\", \"/MDd\"])\n\t"
                                          "else:\n\t\t"
                                          "env.Append(CCFLAGS=[\"-O2\", \"-EHsc\", \"-DNDEBUG\", \"/MDd\"])\n\n"
                                          "for f in os.listdir(dir):\n\t\t"
                                          "if f.endswith(\".cpp\"):\n\t\t\t"
                                          "sources.append(dir + \"/\" + f)\n\n"
                                          "env.Append(\n\t"
                                          "CPPPATH=[\n\t\t"
                                          "godot_headers_path,\n\t\t"
                                          "godot_bindings_path + \"/include\",\n\t\t"
                                          "godot_bindings_path + \"/include/gen/\",\n\t\t"
                                          "godot_bindings_path + \"/include/core/\",\n\t"
                                          "],\n)"
                                          "if target == \"debug\":\n\t"
                                          "env.Append(LIBS=[\"libgodot-cpp." +
                               scons_platform + ".debug.64\"])\n"
                                                "else:\n\t"
                                                "env.Append(LIBS=[\"libgodot-cpp." +
                               scons_platform + ".release.64\"])\n"
                                                "env.Append(LIBPATH=[godot_bindings_path + \"/bin/\"])\n\n"
                                                "sources = []\n"
                                                "add_sources(sources, \"" +
                               source_folder + ")\n\n"
                                               "library = env.SharedLibrary(target=\"bin/libconstructor\", source=sources)\n"
                                               "Default(library)");
            file->close();

            file->open(path + "/settings.gdnproj", File::WRITE);
            file->store_string("language=c++\n\n"
                               "path=" +
                               path + "\n"
                                      "sources_folder=" +
                               source_folder + "\n\n"
                                               "godot_cpp_folder=" +
                               cpp_path + "\n"
                                          "include_folders=\"\"\n"
                                          "linker_folders=\"\"");
            file->close();

            file->free();
            dir->free();
        }
        break;
    case 1:
        OS *cmd;
        PoolStringArray args;
        String gdn_version = ((LineEdit *)get_node(NodePath("ProjectManager/TabContainer/NewProject/Rust/Version/Version")))->get_text();
        String project_name = ((LineEdit *)get_node(NodePath("ProjectManager/TabContainer/NewProject/Rust/Name/Name")))->get_text();
        args.append("init");
        args.append("--lib");
        args.append(path + "/" + project_name);
        cmd->execute("cargo", args);

        File *file = File::_new();
        file->open(path + "/" + project_name + "/Cargo.toml", File::READ);
        int index = 1;
        String final_string;
        while (file->eof_reached() == false)
        {
            String line = file->get_line();
            if (index == 8)
            {
                line += "[lib]\ncrate-type = [\"cdylib\"]\n";
            }
            if (line == "[dependencies]")
            {
                line += "\ngdnative = \"" + gdn_version + "\"";
            }
            final_string += line;
            final_string += "\n";
            index += 1;
        }
        file->close();

        file->open(path + "/" + project_name + "/Cargo.toml", File::WRITE);
        file->store_string(final_string);
        file->close();

        file->open(path + project_name + "/settings.gdnproj", File::WRITE);
        file->store_string("language=rust\n\n"
                           "gnative_version = " +
                           gdn_version + "\n"
                                         "path=" +
                           path + project_name + "\n");
        file->close();

        file->free();
        break;
    }
}

void EditorFile::_register_methods()
{
    register_method((char *)"_init", &EditorFile::_init);
    register_method((char *)"_ready", &EditorFile::_ready);
    register_method((char *)"on_file_pressed", &EditorFile::on_file_pressed);
    register_method((char *)"open_file", &EditorFile::open_file);
    register_method((char *)"save_file", &EditorFile::save_file);
    register_method((char *)"create_shortcuts", &EditorFile::create_shortcuts);
    register_method((char *)"create_new_class", &EditorFile::create_new_class);
    register_method((char *)"create_new_project", &EditorFile::create_new_project);

    register_method((char *)"_on_NewFile_file_selected", &EditorFile::_on_NewFile_file_selected);
    register_method((char *)"_on_OpenFile_file_selected", &EditorFile::_on_OpenFile_file_selected);
    register_method((char *)"_on_TabContainer_tab_changed", &EditorFile::_on_TabContainer_tab_changed);
    register_method((char *)"_on_TabContainer_tab_close", &EditorFile::_on_TabContainer_tab_close);
    register_method((char *)"_on_CancelButton_pressed", &EditorFile::_on_CancelButton_pressed);
    register_method((char *)"_on_NewClassButton_pressed", &EditorFile::_on_NewClassButton_pressed);
    register_method((char *)"_on_FolderPath_dir_selected", &EditorFile::_on_FolderPath_dir_selected);
    register_method((char *)"_on_ClassName_text_changed", &EditorFile::_on_ClassName_text_changed);
    register_method((char *)"_on_OkButton_pressed", &EditorFile::_on_OkButton_pressed);
    register_method((char *)"_on_ProjectType_item_selected", &EditorFile::_on_ProjectType_item_selected);
    register_method((char *)"_on_PathButton_pressed", &EditorFile::_on_PathButton_pressed);
    register_method((char *)"_on_SearchCPPButton_pressed", &EditorFile::_on_SearchCPPButton_pressed);
    register_method((char *)"_on_cppPathSearch_dir_selected", &EditorFile::_on_cppPathSearch_dir_selected);
    register_method((char *)"_process", &EditorFile::_process);
}

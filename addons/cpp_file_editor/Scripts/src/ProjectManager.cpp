#include <Godot.hpp>
#include <Control.hpp>
#include <File.hpp>
#include <Directory.hpp>
#include <OS.hpp>
#include <TabContainer.hpp>
#include <LineEdit.hpp>
#include <OptionButton.hpp>
#include <AcceptDialog.hpp>
#include <FileDialog.hpp>
#include <WindowDialog.hpp>

#include "ProjectManager.hpp"
#include "FileManager.hpp"
#include "CodeEditor.hpp"

using namespace godot;

ProjectManager::ProjectManager()
{
}

ProjectManager::~ProjectManager()
{
    if (thread != nullptr)
    {
        thread->join();
        delete thread;
    }
}

void ProjectManager::_init()
{
}

void ProjectManager::_ready()
{
}

void ProjectManager::build_task(int task = 0)
{
    if (cast_to<EditorFile>(this->get_parent())->get_project_path() == "")
    {
        ((WindowDialog *)get_node(NodePath("BuildWarning")))->popup_centered();
    }
    else
    {
        PoolStringArray keys = Array::make("language", "path", "build_command", "clean_command");
        String load_file = cast_to<EditorFile>(this->get_parent())->get_project_path() + "/settings.gdnproj";
        PoolStringArray settings = cast_to<EditorFile>(this->get_parent())->load_config(load_file, "settings", keys);
        String execute_command = settings[2];
        if (task == 1)
        {
            execute_command = settings[3];
        }

        this->check_thread();
        String selected_os = cast_to<EditorFile>(this->get_parent())->get_selected_platform();

        if (settings[0] == "c++")
        {
            String command = this->build_cpp_project(settings[1], selected_os, execute_command);
            thread = new std::thread(&EditorFile::execute_command, cast_to<EditorFile>(this->get_parent()), command);
        }
        else if (settings[0] == "rust")
        {
            String command = this->build_rust_project(settings[1], selected_os, execute_command);
            thread = new std::thread(&EditorFile::execute_command, cast_to<EditorFile>(this->get_parent()), command);
        }
    }
}

String ProjectManager::build_cpp_project(String path, String selected_platform, String command_line)
{
    PoolStringArray keys = Array::make("godot_cpp_folder", "include_folders", "linker_folders", "linker_settings");
    String load_file = cast_to<EditorFile>(this->get_parent())->get_project_path() + "/settings.gdnproj";
    PoolStringArray settings = cast_to<EditorFile>(this->get_parent())->load_config(load_file, "settings", keys);
    String bindings = settings[0];
    String include_folders = settings[1];
    String linker_folders = settings[2];
    String linker_settings = settings[3];

    String command = command_line;
    command = command.replace("{platform}", selected_platform);
    command = command.replace("{path}", path);
    command = command.replace("{bindings_path}", bindings);
    command = command.replace("{include}", include_folders);
    command = command.replace("{linker}", linker_folders);
    command = command.replace("{libs}", linker_settings);
    if (cast_to<EditorFile>(this->get_parent())->get_selected_profile() == true)
    {
        command += "target=release";
    }

    return command;
}

String ProjectManager::build_rust_project(String path, String selected_platform, String command_line)
{
    String command = command_line;
    command = command.replace("{path}", path + "/Cargo.toml");
    String os_name = OS::get_singleton()->get_name();

    if (selected_platform == "windows")
    {
        if (os_name != "windows")
        {
            command = command.replace("{platform}", "x86_64-pc-windows-msvc");
        }
    }
    else if (selected_platform == "x11")
    {
        if (os_name != "x11")
        {
            command = command.replace("{platform}", "x86_64-unknown-linux-gnu");
        }
    }
    else if (selected_platform == "osx")
    {
        if (os_name != "osx")
        {
            command = command.replace("{platform}", "x86_64-apple-darwin");
        }
    }

    if (cast_to<EditorFile>(this->get_parent())->get_selected_profile() == true)
    {
        command += "--release";
    }

    return command;
}

void ProjectManager::create_new_class()
{
    String class_path = ((LineEdit *)get_node(NodePath("TabContainer/NewClass/PathLabel/FilePath")))->get_text();
    String class_name = ((LineEdit *)get_node(NodePath("TabContainer/NewClass/ClassName/ClassName")))->get_text();
    String main_class = ((LineEdit *)get_node(NodePath("TabContainer/NewClass/MainLib/MainPath")))->get_text();
    int inherit_item = ((OptionButton *)get_node(NodePath("TabContainer/NewClass/Inherit/Inherits")))->get_selected();
    String class_inherit = ((OptionButton *)get_node(NodePath("TabContainer/NewClass/Inherit/Inherits")))->get_item_text(inherit_item);
    int class_lang = ((OptionButton *)get_node(NodePath("TabContainer/NewClass/ClassType/ClassType")))->get_selected_id();

    if (class_name == "" || class_path == "")
    {
        ((AcceptDialog *)get_node(NodePath("TabContainer/NewClass/Warning")))->popup_centered();
    }
    else
    {
        File *file = File::_new();
        if (class_lang == 0)
        {
            file->open(class_path + "/" + class_name + ".hpp", File::WRITE);
            file->store_string("#include <Godot.hpp>\n#include <" + class_inherit + ".hpp>\n\nclass " + class_name + " : public " + class_inherit +
                               "{\n\tGODOT_CLASS(" + class_name + "," + class_inherit + ")" +
                               "\n\nprivate:\n\npublic:\n\tstatic void _register_methods();\n\t" + class_name + "();\n\t~" +
                               class_name + "();\n\n};");
            file->close();
            file->open(class_path + "/" + class_name + ".cpp", File::WRITE);
            file->store_string("#include \"" + class_name + ".hpp\"\n\nusing namespace godot;\n\nvoid " +
                               class_name + "::_register_methods() {\n\tregister_method(\"_process\", &" +
                               class_name + "::_process);\n}\n\n" + class_name + "::" + class_name +
                               "() {\n}\n\n" + class_name + "::~" + class_name + "() {\n}");
            file->close();
            if (main_class != "")
            {
                file->open(main_class, File::READ);

                int index = 1;
                String final_string;
                while (file->eof_reached() == false)
                {
                    String line = file->get_line();
                    if (line == "#include <Godot.hpp>")
                    {
                        line += "\n\n#include \"" + class_name + ".hpp\"";
                    }
                    else if (line == "\tgodot::Godot::nativescript_init(handle);")
                    {
                        line += "\n\n\tregister_class<" + class_name + ">()\n";
                    }
                    else
                    {
                        line += "\n";
                    }
                    final_string += line;
                    index += 1;
                }
                file->close();

                file->open(main_class, File::WRITE);
                file->store_string(final_string);
                file->close();
            }
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

void ProjectManager::create_new_project()
{
    String path = ((LineEdit *)get_node(NodePath("TabContainer/NewProject/PathLabel/FilePath")))->get_text();
    String cpp_path = ((LineEdit *)get_node(NodePath("TabContainer/NewProject/CPP/cppPath/cppPath")))->get_text();

    switch (((OptionButton *)get_node(NodePath("TabContainer/NewProject/ProjectType/ProjectType")))->get_selected_id())
    {
    case 0:
        if (path == "" || cpp_path == "")
        {
            ((AcceptDialog *)get_node(NodePath("TabContainer/NewProject/CPP/Warning")))->popup_centered();
        }
        else
        {
            String current_platform = "";
            String get_platform = OS::get_singleton()->get_name();

            String source_folder = ((LineEdit *)get_node(NodePath("TabContainer/NewProject/CPP/Source/Source")))->get_text();
            String project_name = ((LineEdit *)get_node(NodePath("TabContainer/NewProject/CPP/Name/Name")))->get_text();

            if (get_platform == "Windows")
            {
                current_platform = "windows";
            }
            else if (get_platform == "X11")
            {
                current_platform = "linux";
            }
            else if (get_platform == "OSX")
            {
                current_platform = "osx";
            }

            File *file = File::_new();
            Directory *dir = Directory::_new();
            dir->open(path);
            dir->make_dir(project_name);
            dir->open(path + "/" + project_name);
            dir->make_dir(source_folder);
            file->open(path + "/" + project_name + "/" + source_folder + "/main.cpp", File::WRITE);
            file->store_string("#include <Godot.hpp>\n\n"
                               "extern \"C\" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {\n\t"
                               "godot::Godot::gdnative_init(o);\n}\n\n"
                               "extern \"C\" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {\n\t"
                               "godot::Godot::gdnative_terminate(o);\n}\n\n"
                               "extern \"C\" void GDN_EXPORT godot_nativescript_init(void *handle) {\n\t"
                               "godot::Godot::nativescript_init(handle);\n}");
            file->close();
            file->open(path + "/" + project_name + "/SConstruct", File::WRITE);
            file->store_string("import os\n"
                               "import platform as pl\n\n"
                               "current_platform = \"windows\"\n"
                               "if pl.system() == \"Linux\":\n\t"
                               "current_platform = \"linux\"\n"
                               "elif pl.system() == \"Darwin\":\n\t"
                               "current_platform = \"osx\"\n\n"
                               "platform = ARGUMENTS.get(\"p\", \"linux\")\n"
                               "platform = ARGUMENTS.get(\"platform\", platform)\n\n"
                               "bindings = \"\"\n"
                               "bindings = ARGUMENTS.get(\"cpp_path\", bindings)\n\n"
                               "include_folders = \"\"\n"
                               "include_folders = ARGUMENTS.get(\"include_path\", include_folders)\n\n"
                               "linker_folders = \"\"\n"
                               "linker_folders = ARGUMENTS.get(\"linker_path\", linker_folders)\n\n"
                               "linker_settings = \"\"\n"
                               "linker_settings = ARGUMENTS.get(\"link_libs\", linker_settings)\n\n"
                               "env = Environment()\n"
                               "if platform == \"windows\":\n\t"
                               "env = Environment(ENV=os.environ)\n\n"
                               "godot_headers_path = ARGUMENTS.get(\"headers\", os.getenv(\"GODOT_HEADERS\", bindings+\"/godot_headers\"))\n"
                               "godot_bindings_path = ARGUMENTS.get(\"cpp_bindings\", os.getenv(\"CPP_BINDINGS\", bindings))\n\n"
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
                               "def add_sources(sources, dir):\n\t"
                               "for f in os.listdir(dir):\n\t\t"
                               "if f.endswith(\".cpp\"):\n\t\t\t"
                               "sources.append(dir + \"/\" + f)\n\n"
                               "env.Append(\n\t"
                               "CPPPATH=[\n\t\t"
                               "godot_headers_path,\n\t\t"
                               "godot_bindings_path + \"/include\",\n\t\t"
                               "godot_bindings_path + \"/include/gen/\",\n\t\t"
                               "godot_bindings_path + \"/include/core/\",\n\t"
                               "],\n)\n\n"
                               "if target == \"debug\":\n\t"
                               "env.Append(LIBS=[\"libgodot-cpp.\"+current_platform+\".debug.64\"])\n"
                               "else:\n\t"
                               "env.Append(LIBS=[\"libgodot-cpp.\"+current_platform+\".release.64\"])\n\n"
                               "env.Append(LIBPATH=[godot_bindings_path + \"/bin/\"])\n\n"
                               "for includes in include_folders.split(\",\"):\n\t"
                               "env.Append(CPPPATH=[includes])\n\n"
                               "for linker in linker_folders.split(\",\"):\n\t"
                               "env.Append(LIBPATH=[linker])\n\n"
                               "for lib in linker_settings.split(\",\"):\n\t"
                               "env.Append(LIBS=[lib])\n\n"
                               "sources = []\n"
                               "add_sources(sources, \"" +
                               source_folder + "\")\n\n"
                                               "library = env.SharedLibrary(target=\"bin/lib" +
                               project_name + "\", source=sources)\n"
                                              "Default(library)");
            file->close();

            file->open(path + "/" + project_name + "/settings.gdnproj", File::WRITE);
            file->store_string("[settings]\n"
                               "language=\"c++\"\n\n"
                               "path=\"" +
                               path + "/" + project_name + "\"\n"
                                                           "sources_folder=\"" +
                               source_folder + "\"\n\n"
                                               "godot_cpp_folder=\"" +
                               cpp_path + "\"\n"
                                          "include_folders=\"\"\n"
                                          "linker_folders=\"\"\n"
                                          "linker_settings=\"\"\n"
                                          "build_command=\"scons -C {path} platform={platform} cpp_path={bindings_path} include_path={include} linker_path={linker} link_libs={libs}\"\n"
                                          "clean_command=\"scons -C {path} --clean\"");
            file->close();

            cast_to<EditorFile>(this->get_parent())->open_file(path + "/" + project_name + "/" + source_folder + "/main.cpp");
            cast_to<EditorFile>(this->get_parent())->change_project_path(path + "/" + project_name);
            file->free();
            dir->free();
        }
        cast_to<EditorFile>(this->get_parent())->execute_command("python --version");
        cast_to<EditorFile>(this->get_parent())->execute_command("scons --version");
        break;
    case 1:
        this->check_thread();
        String project_name = ((LineEdit *)get_node(NodePath("TabContainer/NewProject/Rust/Name/Name")))->get_text();

        std::future<void> th = std::async(std::launch::async, &EditorFile::execute_command, cast_to<EditorFile>(this->get_parent()), "cargo init --lib " + path + "/" + project_name);
        while (true)
        {
            if (th.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                this->check_thread();
                this->create_rust_project(path);
                cast_to<EditorFile>(this->get_parent())->open_file(path + "/" + project_name + "/src/lib.rs");
                cast_to<EditorFile>(this->get_parent())->change_project_path(path + "/" + project_name);
                break;
            }
        }
        cast_to<EditorFile>(this->get_parent())->execute_command("cargo --version");
        break;
    }
}

void ProjectManager::create_rust_project(String path)
{
    String gdn_version = ((LineEdit *)get_node(NodePath("TabContainer/NewProject/Rust/Version/Version")))->get_text();
    String project_name = ((LineEdit *)get_node(NodePath("TabContainer/NewProject/Rust/Name/Name")))->get_text();

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

    file->open(path + "/" + project_name + "/src/lib.rs", File::WRITE);
    file->store_string("use gdnative::prelude::*;\n\n"
                       "#[derive(NativeClass)]\n"
                       "#[inherit(Node)]\n"
                       "struct HelloWorld;\n\n"
                       "#[gdnative::methods]\n"
                       "impl HelloWorld {\n\t"
                       "fn new(_owner: &Node) -> Self {\n\t\t"
                       "HelloWorld\n\t"
                       "}\n\n\t"
                       "#[export]\n\t"
                       "fn _ready(&self, _owner: &Node) {\n\t\t"
                       "godot_print!(\"hello, world.\")\n\t"
                       "}\n"
                       "}\n\n"
                       "fn init(handle: InitHandle) {\n\t"
                       "handle.add_class::<HelloWorld>();\n"
                       "}\n\n");
    file->close();

    file->open(path + "/" + project_name + "/settings.gdnproj", File::WRITE);
    file->store_string("[settings]\n"
                       "language=\"rust\"\n\n"
                       "path=\"" +
                       path + "/" + project_name + "\"\n"
                                                   "gdnative_version=\"" +
                       gdn_version + "\"\n"
                                     "build_command=\"cargo build --manifest-path={path}\"\n"
                                     "clean_command=\"cargo clean --manifest-path={path}\"");
    file->close();
    file->free();
}

void ProjectManager::check_thread()
{
    if (thread != nullptr)
    {
        thread->join();
        delete thread;
        thread = nullptr;
    }
}

void ProjectManager::_on_OkButton_pressed()
{
    switch (((TabContainer *)get_node(NodePath("TabContainer")))->get_current_tab())
    {
    case 0:
        this->create_new_class();
        break;
    case 1:
        this->create_new_project();
        break;
    }
    this->hide();
}

void ProjectManager::_on_CancelButton_pressed()
{
    this->hide();
}

void ProjectManager::_on_FolderPath_dir_selected(String path)
{
    File *file = File::_new();
    int class_lang = ((OptionButton *)get_node(NodePath("TabContainer/NewClass/ClassType/ClassType")))->get_selected_id();
    switch (((TabContainer *)get_node(NodePath("TabContainer")))->get_current_tab())
    {
    case 0:
        ((LineEdit *)get_node(NodePath("TabContainer/NewClass/PathLabel/FilePath")))->set_text(path);
        if (class_lang == 0)
        {
            if (file->file_exists(path + "/main.cpp"))
            {
                ((LineEdit *)get_node(NodePath("TabContainer/NewClass/MainLib/MainPath")))->set_text(path + "/main.cpp");
            }
            else
            {
                ((LineEdit *)get_node(NodePath("TabContainer/NewClass/MainLib/MainPath")))->set_text("");
            }
        }
        else if (class_lang == 1)
        {
            if (file->file_exists(path + "/lib.rs"))
            {
                ((LineEdit *)get_node(NodePath("TabContainer/NewClass/MainLib/MainPath")))->set_text(path + "/lib.rs");
            }
            else
            {
                ((LineEdit *)get_node(NodePath("TabContainer/NewClass/MainLib/MainPath")))->set_text("");
            }
        }
        break;
    case 1:
        ((LineEdit *)get_node(NodePath("TabContainer/NewProject/PathLabel/FilePath")))->set_text(path);
        break;
    }
    file->free();
}

void ProjectManager::_on_NewClassButton_pressed()
{
    ((FileDialog *)get_node(NodePath("FolderPath")))->popup_centered();
}

void ProjectManager::_on_ClassName_text_changed(String new_text)
{
    int class_lang = ((OptionButton *)get_node(NodePath("TabContainer/NewClass/ClassType/ClassType")))->get_selected_id();
    switch (class_lang)
    {
    case 0:
        ((LineEdit *)get_node(NodePath("TabContainer/NewClass/Main")))->set_text(new_text + ".cpp");
        ((LineEdit *)get_node(NodePath("TabContainer/NewClass/Include")))->set_text(new_text + ".h");
        break;
    case 1:
        ((LineEdit *)get_node(NodePath("TabContainer/NewClass/Main")))->set_text(new_text + ".rs");
        ((LineEdit *)get_node(NodePath("TabContainer/NewClass/Include")))->set_text("");
        break;
    }
}

void ProjectManager::_on_ProjectType_item_selected(int index)
{
    switch (index)
    {
    case 0:
        ((Control *)get_node(NodePath("TabContainer/NewProject/Rust")))->hide();
        ((Control *)get_node(NodePath("TabContainer/NewProject/CPP")))->show();
        break;
    case 1:
        ((Control *)get_node(NodePath("TabContainer/NewProject/CPP")))->hide();
        ((Control *)get_node(NodePath("TabContainer/NewProject/Rust")))->show();
        break;
    }
}

void ProjectManager::_on_PathButton_pressed()
{
    ((FileDialog *)get_node(NodePath("FolderPath")))->popup_centered();
}

void ProjectManager::_on_SearchCPPButton_pressed()
{
    ((FileDialog *)get_node(NodePath("TabContainer/NewProject/CPP/cppPathSearch")))->popup_centered();
}

void ProjectManager::_on_cppPathSearch_dir_selected(String path)
{
    ((LineEdit *)get_node(NodePath("TabContainer/NewProject/CPP/cppPath/cppPath")))->set_text(path);
}

void ProjectManager::_register_methods()
{
    register_method((char *)"_init", &ProjectManager::_init);
    register_method((char *)"_ready", &ProjectManager::_ready);
    register_method((char *)"build_task", &ProjectManager::build_task);
    register_method((char *)"build_cpp_project", &ProjectManager::build_cpp_project);
    register_method((char *)"build_rust_project", &ProjectManager::build_rust_project);
    register_method((char *)"create_new_class", &ProjectManager::create_new_class);
    register_method((char *)"create_new_project", &ProjectManager::create_new_project);
    register_method((char *)"create_rust_project", &ProjectManager::create_rust_project);
    register_method((char *)"check_thread", &ProjectManager::check_thread);

    register_method((char *)"_on_OkButton_pressed", &ProjectManager::_on_OkButton_pressed);
    register_method((char *)"_on_CancelButton_pressed", &ProjectManager::_on_CancelButton_pressed);
    register_method((char *)"_on_FolderPath_dir_selected", &ProjectManager::_on_FolderPath_dir_selected);
    register_method((char *)"_on_NewClassButton_pressed", &ProjectManager::_on_NewClassButton_pressed);
    register_method((char *)"_on_ClassName_text_changed", &ProjectManager::_on_ClassName_text_changed);
    register_method((char *)"_on_ProjectType_item_selected", &ProjectManager::_on_ProjectType_item_selected);
    register_method((char *)"_on_PathButton_pressed", &ProjectManager::_on_PathButton_pressed);
    register_method((char *)"_on_SearchCPPButton_pressed", &ProjectManager::_on_SearchCPPButton_pressed);
    register_method((char *)"_on_cppPathSearch_dir_selected", &ProjectManager::_on_cppPathSearch_dir_selected);
}
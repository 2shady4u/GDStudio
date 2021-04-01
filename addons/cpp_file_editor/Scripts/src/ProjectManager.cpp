#include <Godot.hpp>
#include <Control.hpp>
#include <File.hpp>
#include <Directory.hpp>
#include <OS.hpp>
#include <ConfigFile.hpp>
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

void ProjectManager::build_task()
{
    ConfigFile *proj_file = ConfigFile::_new();
    if (cast_to<EditorFile>(this->get_parent())->get_project_path() == "")
    {
        ((WindowDialog *)get_node(NodePath("BuildWarning")))->popup_centered();
    }
    else
    {
        proj_file->load(cast_to<EditorFile>(this->get_parent())->get_project_path());
        String lang = proj_file->get_value("settings", "language");
        String path = proj_file->get_value("settings", "path");
        this->check_thread();
        String selected_os = cast_to<EditorFile>(this->get_parent())->get_selected_platform();

        if (lang == "c++")
        {
            thread = new std::thread(&ProjectManager::build_cpp_project, this, path, selected_os);
        }
        else if (lang == "rust")
        {
            thread = new std::thread(&ProjectManager::build_rust_project, this, path, selected_os);
        }
        proj_file->free();
    }
}

void ProjectManager::build_cpp_project(String path, String selected_platform)
{
    PoolStringArray args;
    String platform = "platform=" + selected_platform;
    Array output;
    args.append("-C");
    args.append(path);
    args.append(platform);
    OS::get_singleton()->execute("scons", args, true, output);
    EditorFile *editor = cast_to<EditorFile>(this->get_parent());
    for (int i = 0; i < output.size(); i++)
    {
        editor->get_editor_instance()->edit_log(output[i]);
    }
}

void ProjectManager::build_rust_project(String path, String selected_platform)
{
    PoolStringArray args;
    String os_name = OS::get_singleton()->get_name();

    args.append("build");
    args.append("--manifest-path=" + path + "/Cargo.toml");

    if (selected_platform == "windows")
    {
        if (os_name != "windows")
        {
            args.append("--target=x86_64-pc-windows-msvc");
        }
    }
    else if (selected_platform == "x11")
    {
        if (os_name != "x11")
        {
            args.append("--target=x86_64-unknown-linux-gnu");
        }
    }
    else if (selected_platform == "osx")
    {
        if (os_name != "osx")
        {
            args.append("--target=x86_64-apple-darwin");
        }
    }
    OS::get_singleton()->execute("cargo", args);
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
                        line += "\n\n#include \"" + class_name + ".h\"";
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
            OS *cmd;
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
                                          "env.Append(LIBS=[\"libgodot-cpp." +
                               current_platform + ".debug.64\"])\n"
                                                  "else:\n\t"
                                                  "env.Append(LIBS=[\"libgodot-cpp." +
                               current_platform + ".release.64\"])\n"
                                                  "env.Append(LIBPATH=[godot_bindings_path + \"/bin/\"])\n\n"
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
                                          "linker_folders=\"\"");
            file->close();

            cast_to<EditorFile>(this->get_parent())->change_project_path(path + "/" + project_name + "/settings.gdnproj");
            cast_to<EditorFile>(this->get_parent())->open_file(path + "/" + project_name + "/" + source_folder + "/main.cpp");
            file->free();
            dir->free();
        }
        break;
    case 1:
        this->check_thread();
        thread = new std::thread(&ProjectManager::create_rust_project, this, path);
        break;
    }
}

void ProjectManager::create_rust_project(String path)
{
    PoolStringArray args;
    String gdn_version = ((LineEdit *)get_node(NodePath("TabContainer/NewProject/Rust/Version/Version")))->get_text();
    String project_name = ((LineEdit *)get_node(NodePath("TabContainer/NewProject/Rust/Name/Name")))->get_text();

    args.append("init");
    args.append("--lib");
    args.append(path + "/" + project_name);
    OS::get_singleton()->execute("cargo", args);

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
                       "gnative_version=\"" +
                       gdn_version + "\"\n"
                                     "path=\"" +
                       path + "/" + project_name + "\"\n");
    file->close();
    cast_to<EditorFile>(this->get_parent())->change_project_path(path + "/" + project_name + "/settings.gdnproj");
    cast_to<EditorFile>(this->get_parent())->open_file(path + "/" + project_name + "/src/lib.rs");
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
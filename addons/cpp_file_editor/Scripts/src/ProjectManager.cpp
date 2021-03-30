#include <Godot.hpp>
#include <Control.hpp>
#include <OS.hpp>

#include "ProjectManager.hpp"

using namespace godot;

ProjectManager::ProjectManager()
{
}

ProjectManager::~ProjectManager()
{
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
    for (int i = 0; i < output.size(); i++)
    {
        Godot::print(output[i]);
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

void ProjectManager::register_methods()
{
    register_method((char *)"build_cpp_project", &ProjectManager::build_cpp_project);
    register_method((char *)"build_rust_project", &ProjectManager::build_rust_project);
}
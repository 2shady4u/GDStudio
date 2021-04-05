#include <Godot.hpp>

#include "FileManager.hpp"
#include "CodeEditor.hpp"
#include "ProjectManager.hpp"
#include "Settings.hpp"
#include "ProjectSettings.hpp"

using namespace godot;

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {
	Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {
	Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) {
	Godot::nativescript_init(handle);

	register_class<EditorFile>();
	register_class<CodeEditor>();
	register_class<ProjectManager>();
	register_class<Settings>();
	register_class<ProjectSettings>();

	//register_tool_class<EditorFile>();
	//register_tool_class<CodeEditor>();
}

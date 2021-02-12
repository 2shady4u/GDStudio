#tool
extends EditorPlugin

var editor = preload("res://addons/cpp_file_editor/Scenes/File.tscn")
var editor_instance


func _enter_tree():
	editor_instance = editor.instance()
	get_editor_interface().get_editor_viewport().add_child(editor_instance)
	make_visible(false)

func _exit_tree():
	remove_control_from_docks(editor_instance)
	editor_instance.free()
	get_editor_interface().get_editor_viewport().remove_child(editor_instance)
	#if editor_instance:
	#	editor_instance.queue_free()

func has_main_screen():
	return true

func make_visible(visible):
	if editor_instance:
		editor_instance.visible = true

func get_plugin_name():
	return "C++"

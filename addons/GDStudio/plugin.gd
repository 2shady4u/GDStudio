tool
extends EditorPlugin

var editor_instance

func _enter_tree():
	var editor = load("res://addons/GDStudio/Scenes/File.tscn")
	editor_instance = editor.instance()
	get_editor_interface().get_editor_viewport().add_child(editor_instance)
	make_visible(false)

func _exit_tree():
	get_editor_interface().get_editor_viewport().remove_child(editor_instance)
	if editor_instance:
		editor_instance.queue_free()
	editor_instance.free()

func has_main_screen():
	return true

func make_visible(visible):
	if editor_instance:
		editor_instance.visible = true

func get_plugin_name():
	return "CodeEditor"

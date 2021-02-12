#tool
extends Control
# Declare member variables here. Examples:
# var a = 2
# var b = "text"
var file_name = ""
var file_path = ""
var code_scene = preload("res://addons/cpp_file_editor/Scenes/CodeEditor.tscn")
var current_editor_instance
var current_scene
var tab_number = 0
var time: int

# Called when the node enters the scene tree for the first time.
func _ready():
	open_file("res://godot.cpp")
	file_path = "res://godot.cpp"
	file_name = file_path.split("//")[1]
	current_editor_instance = $TabContainer.get_child(0)
	$TopBar/MenuButton.get_popup().connect("id_pressed",self,"_on_file_pressed")
	create_shortcuts()

func _on_file_pressed(index: int):
	match index:
		0:
			$NewFile.popup_centered()
		1:
			save_file()
		2:
			pass
		3:
			$OpenFile.popup_centered()
		_:
			pass

func open_file(path):
	var file = File.new()
	file.open(path, File.READ)
	var new_code_instance = code_scene.instance()
	var content = file.get_as_text()
	file.close()
	$TabContainer.add_child(new_code_instance, true)
	new_code_instance.set_initial_content(content)
	tab_number = $TabContainer.get_child_count()
	var name = path.split("//")[1]
	$TabContainer.set_tab_title(tab_number - 1, name)

func save_file():
	var file = File.new()
	file.open(file_path, File.WRITE)
	file.store_string(current_editor_instance.get_content())
	current_editor_instance.save_contents()
	file.close()

func _on_NewFile_file_selected(path):
	var file = File.new()
	file.open(path, File.WRITE)
	file.close()
	open_file(path)

func _on_OpenFile_file_selected(path):
	open_file(path)

func create_shortcuts():
	var hotkey

	hotkey = InputEventKey.new()
	hotkey.scancode = KEY_S
	hotkey.control = true
	$TopBar/MenuButton.get_popup().set_item_accelerator(2,hotkey.get_scancode_with_modifiers()) # save file

	hotkey = InputEventKey.new()
	hotkey.scancode = KEY_N
	hotkey.control = true
	$TopBar/MenuButton.get_popup().set_item_accelerator(0,hotkey.get_scancode_with_modifiers()) # new file

	hotkey = InputEventKey.new()
	hotkey.scancode = KEY_S
	hotkey.control = true
	hotkey.shift = true
	$TopBar/MenuButton.get_popup().set_item_accelerator(3,hotkey.get_scancode_with_modifiers()) # save as file

	hotkey = InputEventKey.new()
	hotkey.scancode = KEY_O
	hotkey.control = true
	$TopBar/MenuButton.get_popup().set_item_accelerator(5,hotkey.get_scancode_with_modifiers()) # open file

func _on_TabContainer_tab_changed(tab: int):
	file_path = $TabContainer.get_tab_title(tab)
	file_name = file_path
	current_editor_instance = $TabContainer.get_child(tab)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	if current_editor_instance.text_changed == true:
		$TabContainer.set_tab_title($TabContainer.current_tab, file_name + "(*)")
	else:
		$TabContainer.set_tab_title($TabContainer.current_tab, file_name)

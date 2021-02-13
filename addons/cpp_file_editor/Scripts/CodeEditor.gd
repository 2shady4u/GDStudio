#tool
extends Control

# Declare member variables here. Examples:
# var a = 2
# var b = "text"
var time = 0
var line_number = 0
var current_line = 0
var preprocessor = ["#"]
var keywords = ["auto", "short", "struct", "unsigned",
"break", "continue", "else", "for", "long", "signed", "switch", "void",
"case", "default", "enum", "goto", "register", "sizeof", "typedef", "volatile",
"do", "extern", "if", "return", "static", "union", "while",
"asm", "dynamic_cast", "namespace", "reinterpret_cast", "try",
"bool", "explicit", "new", "static_cast", "typeid",
"catch", "false", "operator", "template", "typename",
"class", "friend", "private", "this", "using",
"const_cast", "inline", "public", "throw", "virtual",
"delete", "mutable", "protected", "true", "wchar_t",
"const", "int", "float", "double", "char", "string"]
var error_type = -1
var error_messages = ["Expected ; at the end of the statement", 
"Variable type mismatch"]
var token_line_error = ""
var operators = ["+", "-", "*", "/", "=", "%", "<<", ">>"]
var variable_tracker = []
var error_text = ""
var current_content = ""
var file_name = ""
var in_function = false
var text_changed = false
var has_error = false
var showing_error = false
var error_line = -1

# Called when the node enters the scene tree for the first time.
func _ready():
	$Container/CodeEditor.get_meta("custom_colors")

func set_initial_content(content):
	$Container/CodeEditor.text = content
	current_content = content
	setup_syntax()

func setup_syntax():
	$Container/CodeEditor.add_color_region("\"", "\"",Color8(128, 64, 0,255),true)
	$Container/CodeEditor.add_color_region("//", "",Color8(0, 192, 64,255),true)
	for i in preprocessor:
		$Container/CodeEditor.add_color_region(i, "",Color8(0, 128, 64,255),true)
	
	for i in keywords:
		$Container/CodeEditor.add_keyword_color(i, Color8(128,0,255,255))
	

func get_content():
	return $Container/CodeEditor.text

func save_contents():
	current_content = $Container/CodeEditor.text
	text_changed = false

func _on_CodeEditor_text_changed():
	if $Container/CodeEditor.text == current_content:
		text_changed = false
	else:
		text_changed = true
	line_number = $Container/CodeEditor.get_line_count()
	variable_tracker = []
	$VBoxContainer/VariableLog/VariableLog.text = "Variables"
	for i in line_number:
		tokenize(i)
		check_variables(i)
	if error_type > -1:
		check_errors(token_line_error, error_line)
	show_errors()

func parse_line(line: int):
	var current_line = $Container/CodeEditor.get_line(line)
	var comments = current_line.find("//", 0)
	if comments > -1:
		current_line.erase(comments, 999)
	var correct_line = current_line.strip_edges()
	return correct_line

func tokenize(line):
	var correct_line = parse_line(line)
	var line_split = correct_line.split(" ")
	var token = ""
	var is_string = false
	for value in line_split:
		if is_string == true:
			token += "s"
			if value.ends_with("\""):
				is_string = false
		else:
			if value.begins_with("\""):
				is_string = true
				token += "s"
			if value.begins_with("#"):
				token += "p"
			elif keywords.has(value):
				token += "k"
			elif operators.has(value):
				token += "o"
			elif value.is_valid_integer() or value.is_valid_float():
				token += "n"
			else:
				if $Container/CodeEditor.get_line(line) != "":
					token += "c"
	#print(line)
	#print(token)
	if has_error == false:
		#final semicolon
		var code = token.find("c", 0)
		if code > 0:
			if token[code - 1] == "k" and not correct_line[correct_line.length() - 1].ends_with(")"):
				if correct_line[correct_line.length() - 1] != ";":
					has_error = true
					error_line = line
					error_type = 0
					token_line_error = token
		#variable type
		code = token.find("c", 0)
		if token.length() > code + 1:
			if token[code + 1] == "o":
				for i in variable_tracker:
					if i[1] == line_split[code]:
						var data_type = ""
						if line_split.size() >= code + 3:
							if token[code + 2] != "c":
								var final_line = line_split[code + 2]
								var semicolon = line_split[code + 2].find(";", 0)
								if semicolon > -1:
									final_line.erase(semicolon, 999)
								if final_line.is_valid_integer():
									data_type = "int"
								elif final_line.is_valid_float():
									data_type = "float"
									if i[0] == "double":
										data_type = "double"
								else:
									data_type = "string"
								if data_type != i[0]:
									has_error = true
									error_line = line
									error_type = 1
									token_line_error = token

func check_errors(token: String, line: int):
	match error_type:
		0:
			var correct_line = parse_line(line)
			var code = token.find("c", 0)
			if token[code - 1] == "k":
				if correct_line[correct_line.length() - 1] == ";":
					has_error = false
					error_line = -1
					error_type = -1
					token_line_error = ""
		1:
			var correct_line = parse_line(line)
			var line_split = correct_line.split(" ")
			var code = token.find("c", 0)
			if token.length() > code + 1:
				if token[code + 1] != "o":
					for i in variable_tracker:
						if i[1] == line_split[code]:
							var data_type = ""
							if line_split.size() >= code + 3:
								if token[code + 2] == "c":
									var final_line = line_split[code + 2]
									var semicolon = line_split[code + 2].find(";", 0)
									if semicolon > -1:
										final_line.erase(semicolon, 999)
									if final_line.is_valid_integer():
										data_type = "int"
									elif final_line.is_valid_float():
										data_type = "float"
										if i[0] == "double":
											data_type = "double"
									else:
										data_type = "string"
									if data_type == i[0]:
										has_error = false
										error_line = -1
										error_type = -1
										token_line_error = ""

func show_errors():
	if has_error == true:
		if showing_error == false:
			var error_pos = error_line - $Container/CodeEditor.scroll_vertical
			$Container/CodeEditor/ErrorColor.set_position(Vector2(0, 2 + (error_pos * 20)))
			$Container/CodeEditor/ErrorColor.show()
			error_text = "\n"+error_messages[error_type]
			$VBoxContainer/ErrorLog/ErrorLog.text += error_text
			showing_error = true
	else:
		if showing_error == true:
			$Container/CodeEditor/ErrorColor.hide()
			$Container/CodeEditor/ErrorColor.set_position(Vector2(0, 2 + (error_line * 20)))
			var pos = $VBoxContainer/ErrorLog/ErrorLog.text.find(error_text)
			var length = error_text.length()
			var text = $VBoxContainer/ErrorLog/ErrorLog.text
			text.erase(pos, length)
			$VBoxContainer/ErrorLog/ErrorLog.text = text
			error_text = ""
			showing_error = false

func check_variables(line):
	var current_line = $Container/CodeEditor.get_line(line)
	var correct_line = current_line.strip_edges()
	var line_split = correct_line.split(" ")
	if line_split.size() > 2:
		if keywords.has(line_split[0]) and operators.has(line_split[2]):
			var variable_data = [line_split[0], line_split[1]]
			if find_arrays(variable_data, variable_tracker) == false:
				variable_tracker.append(variable_data)
				$VBoxContainer/VariableLog/VariableLog.text += "\n" + variable_data[1] + ": " + variable_data[0]

func find_arrays(search_array: Array, main_array: Array):
	var total = search_array.size()
	var current = 0
	for sub_array in main_array:
		for i in sub_array.size():
			if search_array[i] == sub_array[i]:
				current += 1
		if current == total:
			return true
		else:
			current = 0
	return false

func auto_add():
	if $Container/CodeEditor.get_word_under_cursor() == "(":
		print("yes")

# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	time += delta
#	if time >= 0.8:
#		time = 0

#tool
extends Control

# Declare member variables here. Examples:
# var a = 2
# var b = "text"
var line_index = 0
var line_number = 0
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
var operators = ["+", "-", "*", "/", "=", "%", "<<", ">>"]
var variable_tracker = []
var error_text = ""
var current_content = ""
var file_name = ""
var text_changed = false
var has_error = false
var error_line = -1
enum variable_types {
	plus,
	minus,
	mult,
	divide,
	equal,
	lparent,
	rparent,
	lbrace,
	rbrace,
	intType,
	floatType,
	doubleType,
	number,
	keyword,
	identifier,
	semicolon,
	comment,
	illegal,
}

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

func check_line(line: int):
	check_variables(line)
	token_line(line)

func token_line(line: int):
	line_index = 0
	var current_line = $Container/CodeEditor.get_line(line)
	var correct_line = current_line.strip_edges()
	var token_return = []
	var chara = ""
	while correct_line.length() > 0:
		chara = correct_line[line_index]
		if chara != " ":
			match chara:
				"+":
					token_return.append([variable_types.plus, "+"])
				"-":
					token_return.append([variable_types.minus, "-"])
				"*":
					token_return.append([variable_types.mult, "*"])
				"/":
					token_return.append([variable_types.divide, "/"])
				"=":
					token_return.append([variable_types.equal, "="])
				"(":
					token_return.append([variable_types.lparent, "("])
				")":
					token_return.append([variable_types.rparent, ")"])
				";":
					token_return.append([variable_types.semicolon, ";"])
				_:
					if chara.is_valid_integer() == true:
						var number = get_whole_integer(correct_line)
						token_return.append([variable_types.number, number])
					elif is_letter(chara):
						var identifier = get_identifier(correct_line)
						if keywords.has(identifier):
							token_return.append([variable_types.keyword, identifier])
						else:
							token_return.append([variable_types.identifier, identifier])
					else:
						token_return.append([variable_types.illegal, chara])
		line_index += 1
		if line_index > correct_line.length() - 1:
			break
	for i in token_return:
		print(i)
	parse_line_tokens(token_return, line)

func get_whole_integer(line: String):
	var position = line_index
	var final_position = line_index
	while line[final_position].is_valid_integer() == true:
		final_position += 1
		if final_position > line.length() - 1:
			break
	var size = final_position - position
	line_index += size - 1
	return line.substr(position, size)

func get_identifier(line: String):
	var position = line_index
	var final_position = line_index
	while is_letter(line[final_position]) == true:
		final_position += 1
		if final_position > line.length() - 1:
			break
	var size = final_position - position
	line_index += size - 1
	return line.substr(position, size)

func is_letter(chara):
	return 'a' <= chara && chara <= 'z' || 'A' <= chara && chara <= 'z' || chara == '_'

func parse_line_tokens(token_array: Array, line: int):
	var error = 1
	match token_array[0][0]:
		variable_types.keyword:
			error = parse_declaration(token_array)
	
	if error == 0:
		has_error = true
		error_line = line
		show_error_text()
	else:
		has_error = false
		error_line = -1
		hide_error_text()

func parse_declaration(token_array: Array):
	var current_token = 1
	while token_array[current_token][0] == variable_types.keyword:
		current_token += 1
	
	if token_array[current_token][0] != variable_types.identifier:
		error_text = "error, expected identifier"
		return 0
	
	current_token += 1
	if token_array[current_token][0] != variable_types.equal:
		error_text = "error, expected = sign"
		return 0
	
	current_token += 1
	while token_array[current_token][0] != variable_types.semicolon:
		print(token_array.size())
		current_token += 1
		if token_array.size() <= current_token:
			error_text = "error, expected ;"
			return 0
	
	return 1

func show_error_text():
	var error_pos = error_line - $Container/CodeEditor.scroll_vertical
	$Container/CodeEditor/ErrorColor.set_position(Vector2(0, 2 + (error_pos * 20)))
	$Container/CodeEditor/ErrorColor.show()
	$VBoxContainer/ErrorLog/ErrorLog.text += "\n"+error_text

func hide_error_text():
	$Container/CodeEditor/ErrorColor.hide()
	$Container/CodeEditor/ErrorColor.set_position(Vector2(0, 2 + (error_line * 20)))
	var pos = $VBoxContainer/ErrorLog/ErrorLog.text.find(error_text)
	var length = error_text.length()
	var text = $VBoxContainer/ErrorLog/ErrorLog.text
	text.erase(pos, length)
	$VBoxContainer/ErrorLog/ErrorLog.text = text
	error_text = ""

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

func _input(event):
	if event is InputEventKey and event.pressed:
		if event.scancode == KEY_HOME:
			var line = $Container/CodeEditor.cursor_get_line()
			token_line(line)

# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass

#include <Godot.hpp>
#include <Control.hpp>
#include <Array.hpp>
#include <TextEdit.hpp>
#include <Color.hpp>
#include <InputEvent.hpp>
#include <InputEventKey.hpp>
#include <GlobalConstants.hpp>
#include <Input.hpp>
#include <OptionButton.hpp>

#include "CodeEditor.hpp"
#include "FileManager.hpp"
using namespace godot;

CodeEditor::CodeEditor()
{
}

CodeEditor::~CodeEditor()
{
}

void CodeEditor::_init()
{
}

void CodeEditor::_ready()
{
    ((TextEdit *)get_node("Container/CodeEditor"))->get_meta("custom_colors");
}

void CodeEditor::set_initial_content(String content)
{
    ((TextEdit *)get_node("Container/CodeEditor"))->set_text(content);
    this->current_content = content;
    this->setup_syntax();
    ((TextEdit *)get_node("Container/CodeEditor"))->cursor_set_line(0);
    ((TextEdit *)get_node("Container/CodeEditor"))->cursor_set_column(0);
}

void CodeEditor::setup_syntax()
{
    ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region("\"", "\"", Color(0.5, 0.5, 0.5, 1), false);
    ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region("//", "", Color(0, 0.5, 0, 1), true);
    ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region("/*", "*/", Color(0, 0.5, 0, 1));
}

void CodeEditor::setup_language(String lang)
{
    if (lang == "cpp")
    {
        Array preprocessor = Array::make("#", "include", "define");
        Array keywords = Array::make("auto", "short", "struct", "unsigned",
                                     "break", "continue", "else", "for", "long", "signed", "switch", "void",
                                     "case", "default", "enum", "goto", "register", "sizeof", "typedef", "volatile",
                                     "do", "extern", "if", "return", "static", "union", "while",
                                     "asm", "dynamic_cast", "namespace", "reinterpret_cast", "try",
                                     "bool", "explicit", "new", "static_cast", "typeid",
                                     "catch", "false", "operator", "template", "typename",
                                     "class", "friend", "private", "this", "using",
                                     "const_cast", "inline", "public", "throw", "virtual",
                                     "delete", "mutable", "protected", "true", "wchar_t",
                                     "const", "int", "float", "double", "char", "string");
        Array operators = Array::make("+", "-", "*", "/", "=", "%", "<<", ">>");
        for (int j = 0; j < preprocessor.size(); j++)
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region(preprocessor[j], "", Color(0.5, 0.25, 0, 1), true);
        }
        for (int i = 0; i < keywords.size(); i++)
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(keywords[i], Color(0.5, 0, 0.5, 1));
        }
    }
    else if (lang == "rust")
    {
        Array keywords = Array::make("alignof", "as", "be", "box", "break", "const", "continue", "crate",
                                     "do", "else", "enum", "extern", "false", "fn", "for", "if", "impl", "in", "let", "loop",
                                     "match", "mod", "mut", "offsetof", "once", "priv", "proc", "pub", "pure", "ref", "return", "self",
                                     "sizeof", "static", "struct", "super", "trait", "true", "type", "typeof", "unsafe", "unsized",
                                     "use", "virtual", "while", "yield");
        Array types = Array::make("bool", "char", "f32", "f64", "i8", "i16", "i32", "i64",
                                  "str", "u8", "u16", "u32", "u64", "Self");
        for (int i = 0; i < keywords.size(); i++)
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(keywords[i], Color(0, 0, 0.5, 1));
        }
        for (int i = 0; i < types.size(); i++)
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(types[i], Color(0, 0, 0.5, 1));
        }
    }
}

String CodeEditor::get_content()
{
    return ((TextEdit *)get_node("Container/CodeEditor"))->get_text();
}

void CodeEditor::save_contents()
{
    this->current_content = ((TextEdit *)get_node("Container/CodeEditor"))->get_text();
    this->text_changed = false;
}

void CodeEditor::_on_CodeEditor_text_changed()
{
    if (((TextEdit *)get_node("Container/CodeEditor"))->get_text() == this->current_content)
    {
        this->text_changed = false;
    }
    else
    {
        this->text_changed = true;
    }
}

bool CodeEditor::get_text_changed()
{
    return this->text_changed;
}

void CodeEditor::edit_log(String text)
{
    ((TextEdit *)get_node(NodePath("BuildContainer/Log")))->insert_text_at_cursor(text);
}

void CodeEditor::_on_CodeEditor_gui_input(InputEvent *event)
{
    InputEventKey *event_key = cast_to<InputEventKey>(event);
    if (event_key)
    {
        if (event_key->is_pressed())
        {
            int64_t column = ((TextEdit *)get_node("Container/CodeEditor"))->cursor_get_column();
            int64_t line = ((TextEdit *)get_node("Container/CodeEditor"))->cursor_get_line();
            ((TextEdit *)get_node("Container/CodeEditor"))->select(line, column - 1, line, column);
            String text = ((TextEdit *)get_node("Container/CodeEditor"))->get_selection_text();

            switch (event_key->get_scancode())
            {
            case GlobalConstants::KEY_APOSTROPHE:
                if (text == "'")
                {
                    ((TextEdit *)get_node("Container/CodeEditor"))->insert_text_at_cursor("''");
                }
                else if (text == "\"")
                {
                    ((TextEdit *)get_node("Container/CodeEditor"))->insert_text_at_cursor("\"\"");
                }
                break;
            case GlobalConstants::KEY_9:
                if (text == "(")
                {
                    ((TextEdit *)get_node("Container/CodeEditor"))->insert_text_at_cursor("()");
                }
                break;
            case GlobalConstants::KEY_BRACELEFT:
                if (text == "[")
                {
                    ((TextEdit *)get_node("Container/CodeEditor"))->insert_text_at_cursor("[]");
                }
                else if (text == "{")
                {
                    ((TextEdit *)get_node("Container/CodeEditor"))->insert_text_at_cursor("{}");
                }
                break;
            case GlobalConstants::KEY_KP_ENTER:
            case GlobalConstants::KEY_ENTER:
                ((TextEdit *)get_node("Container/CodeEditor"))->select(line, column, line, column + 1);
                text = ((TextEdit *)get_node("Container/CodeEditor"))->get_selection_text();
                if (text == "}")
                {
                    int col = 0;
                    String line_text = ((TextEdit *)get_node("Container/CodeEditor"))->get_line(line);
                    String insert_tabs = "\n";
                    while (line_text[col] == '\t')
                    {
                        insert_tabs += "\t";
                        col += 1;
                    }
                    ((TextEdit *)get_node("Container/CodeEditor"))->insert_text_at_cursor("\t");
                    column = ((TextEdit *)get_node("Container/CodeEditor"))->cursor_get_column();
                    ((TextEdit *)get_node("Container/CodeEditor"))->insert_text_at_cursor(insert_tabs + "}");

                    ((TextEdit *)get_node("Container/CodeEditor"))->cursor_set_line(line);
                }
                break;
            }
            ((TextEdit *)get_node("Container/CodeEditor"))->select(line, column, line, column);
            ((TextEdit *)get_node("Container/CodeEditor"))->cursor_set_column(column);
        }
    }
}

void CodeEditor::_on_CodeEditor_symbol_lookup(String symbol, int row, int column)
{
    Godot::print(symbol);
}

String CodeEditor::get_build_platform_cpp()
{
    int index = ((OptionButton *)get_node(NodePath("BuildContainer/Build/Platform/Platform")))->get_selected_id();

    switch (index)
    {
    case 0:
        return "windows";
        break;
    case 1:
        return "x11";
        break;
    case 2:
        return "osx";
        break;
    case 3:
        return "android";
        break;
    case 4:
        return "ios";
        break;
    default:
        return "unknown";
        break;
    }
}

void CodeEditor::_on_Build_pressed()
{
    EditorFile *editor = cast_to<EditorFile>(this->get_parent()->get_parent());
    editor->execute_build();
}

void CodeEditor::_register_methods()
{
    register_method((char *)"_init", &CodeEditor::_init);
    register_method((char *)"set_initial_content", &CodeEditor::set_initial_content);
    register_method((char *)"setup_syntax", &CodeEditor::setup_syntax);
    register_method((char *)"setup_language", &CodeEditor::setup_language);
    register_method((char *)"get_content", &CodeEditor::get_content);
    register_method((char *)"save_contents", &CodeEditor::save_contents);
    register_method((char *)"get_text_changed", &CodeEditor::get_text_changed);
    register_method((char *)"edit_log", &CodeEditor::edit_log);
    register_method((char *)"get_build_platform_cpp", &CodeEditor::get_build_platform_cpp);

    register_method((char *)"_on_CodeEditor_text_changed", &CodeEditor::_on_CodeEditor_text_changed);
    register_method((char *)"_on_CodeEditor_gui_input", &CodeEditor::_on_CodeEditor_gui_input);
    register_method((char *)"_on_Build_pressed", &CodeEditor::_on_Build_pressed);
}

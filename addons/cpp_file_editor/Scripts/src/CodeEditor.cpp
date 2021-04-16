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
#include <Font.hpp>
#include <DynamicFont.hpp>
#include <DynamicFontData.hpp>
#include <Theme.hpp>
#include <LineEdit.hpp>
#include <Texture.hpp>
#include <Reference.hpp>
#include <ResourceLoader.hpp>
#include <String.hpp>
#include <PoolArrays.hpp>
#include <thread>
#include <future>

#include "tree-sitter/tree-sitter.cpp"
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
    ((TextEdit *)get_node("Container/CodeEditor"))->cursor_set_line(0);
    ((TextEdit *)get_node("Container/CodeEditor"))->cursor_set_column(1);
    node_array = test_parse(((TextEdit *)get_node("Container/CodeEditor"))->get_text());
    setup_cpp_colors(node_array);
}

void CodeEditor::setup_language(String lang)
{
    if (lang == "cpp")
    {
        PoolStringArray keys = Array::make("functions", "strings", "comments", "preprocessor", "keywords");
        PoolColorArray cpp_colors_array = cast_to<EditorFile>(this->get_parent())->load_config("user://syntax.cfg", "C++", keys);
        cpp_colors["preproc"] = cpp_colors_array[3];
        cpp_colors["comment"] = cpp_colors_array[2];
        cpp_colors["primitive_type"] = cpp_colors_array[4];
        cpp_colors["type_identifier"] = cpp_colors_array[4];
        cpp_colors["identifier"] = cpp_colors_array[0];
        cpp_colors["number_literal"] = cpp_colors_array[1];
        cpp_colors["string_literal"] = cpp_colors_array[1];
        this->language = "cpp";
    }
    else if (lang == "rust")
    {
        PoolStringArray keys = Array::make("functions", "strings", "comments", "keywords", "types");
        PoolColorArray rust_colors = cast_to<EditorFile>(this->get_parent())->load_config("user://syntax.cfg", "Rust", keys);
        ((TextEdit *)get_node(NodePath("Container/CodeEditor")))->add_color_override("function_color", rust_colors[0]);
        ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region("\"", "\"", rust_colors[1], false);
        ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region("//", "", rust_colors[2], true);
        ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region("/*", "*/", rust_colors[2]);
        Array keywords = Array::make("alignof", "as", "be", "box", "break", "const", "continue", "crate",
                                     "do", "else", "enum", "extern", "false", "fn", "for", "if", "impl", "in", "let", "loop",
                                     "match", "mod", "mut", "offsetof", "once", "priv", "proc", "pub", "pure", "ref", "return", "self",
                                     "sizeof", "static", "struct", "super", "trait", "true", "type", "typeof", "unsafe", "unsized",
                                     "use", "virtual", "while", "yield");
        Array types = Array::make("bool", "char", "f32", "f64", "i8", "i16", "i32", "i64",
                                  "str", "u8", "u16", "u32", "u64", "Self");
        for (int i = 0; i < keywords.size(); i++)
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(keywords[i], rust_colors[3]);
        }
        for (int i = 0; i < types.size(); i++)
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(types[i], rust_colors[4]);
        }
        this->language = "rust";
    }
}

void CodeEditor::setup_cpp_colors(Array node_array)
{
    for (int i = 0; i < node_array.size(); i++)
    {
        Array current_node = node_array[i];
        int start = int(current_node[1]);
        int end = int(current_node[2]);
        int len = int(end - start);
        String keyword = ((TextEdit *)get_node("Container/CodeEditor"))->get_text().substr(start, len);
        Color color = Color(0.0,0.0,0.0,1.0);
        if (cpp_colors.has(current_node[0]))
        {
            color = cpp_colors[current_node[0]];
        }
        String node_name = current_node[0];
        if (current_node[3] != String(""))
        {
            this->setup_cpp_colors(current_node[3]);
        }
        if (node_name.substr(0, 7) == String("preproc"))
        {
            color = cpp_colors["preproc"];
            ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(keyword, color);
        }
        else if (node_name == String("comment"))
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region(keyword, "", color, false);
        }
        else
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(keyword, color);
        }
    }
}

String CodeEditor::get_content()
{
    return ((TextEdit *)get_node("Container/CodeEditor"))->get_text();
}

String CodeEditor::get_language()
{
    return this->language;
}

void CodeEditor::save_contents()
{
    this->current_content = ((TextEdit *)get_node("Container/CodeEditor"))->get_text();
    this->text_changed = false;
}

void CodeEditor::set_custom_font(String path)
{
    Ref<DynamicFontData> font = ResourceLoader::get_singleton()->load(path);
    cast_to<DynamicFont>(*((TextEdit *)get_node("Container/CodeEditor"))->get_font("font"))->set_font_data(font);
}

void CodeEditor::set_font_size(int size)
{
    cast_to<DynamicFont>(*((TextEdit *)get_node("Container/CodeEditor"))->get_font("font"))->set_size(size);
}

void CodeEditor::set_custom_theme(String path)
{
    Ref<Theme> theme = ResourceLoader::get_singleton()->load(path);
    cast_to<Control>(this->get_parent()->get_parent())->set_theme(theme);
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
            node_array = test_parse(((TextEdit *)get_node("Container/CodeEditor"))->get_text());
            if (this->language == "cpp")
            {
                setup_cpp_colors(node_array);
            }
            
        }
    }
}

void CodeEditor::_on_CodeEditor_symbol_lookup(String symbol, int row, int column)
{
    Godot::print(symbol);
}

void CodeEditor::_register_methods()
{
    register_method((char *)"_init", &CodeEditor::_init);
    register_method((char *)"set_initial_content", &CodeEditor::set_initial_content);
    register_method((char *)"setup_language", &CodeEditor::setup_language);
    register_method((char *)"setup_cpp_colors", &CodeEditor::setup_cpp_colors);
    register_method((char *)"get_content", &CodeEditor::get_content);
    register_method((char *)"save_contents", &CodeEditor::save_contents);
    register_method((char *)"get_text_changed", &CodeEditor::get_text_changed);
    register_method((char *)"set_custom_font", &CodeEditor::set_custom_font);
    register_method((char *)"set_font_size", &CodeEditor::set_font_size);
    register_method((char *)"set_custom_theme", &CodeEditor::set_custom_theme);

    register_method((char *)"_on_CodeEditor_text_changed", &CodeEditor::_on_CodeEditor_text_changed);
    register_method((char *)"_on_CodeEditor_gui_input", &CodeEditor::_on_CodeEditor_gui_input);
}

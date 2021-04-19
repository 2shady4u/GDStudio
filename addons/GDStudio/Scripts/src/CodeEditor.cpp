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
#include <VisualServer.hpp>
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
}

void CodeEditor::setup_language(String lang)
{
    if (lang == "cpp")
    {
        PoolStringArray keys = Array::make("identifiers", "namespace", "numbers", "strings", "comments", "preprocessor", "keywords");
        PoolColorArray cpp_colors_array = cast_to<EditorFile>(this->get_parent())->load_config("user://syntax.cfg", "C++", keys);
        cpp_colors["preproc"] = cpp_colors_array[5];
        cpp_colors["comment"] = cpp_colors_array[4];
        cpp_colors["keywords"] = cpp_colors_array[6];
        cpp_colors["identifier"] = cpp_colors_array[0];
        cpp_colors["namespace_identifier"] = cpp_colors_array[1];
        cpp_colors["number_literal"] = cpp_colors_array[2];
        cpp_colors["string_literal"] = cpp_colors_array[3];
        cpp_colors["system_lib_string"] = cpp_colors_array[3];
        for (int i = 0; i < cpp_keywords.size(); i++)
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(cpp_keywords[i], cpp_colors["keywords"]);
        }
        for (int i = 0; i < operators.size(); i++)
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(operators[i], cpp_colors["keywords"]);
        }
        node_array = parse_text(((TextEdit *)get_node("Container/CodeEditor"))->get_text(), lang);
        setup_cpp_colors(node_array);
        this->language = "cpp";
    }
    else if (lang == "rust")
    {
        PoolStringArray keys = Array::make("functions", "strings", "comments", "keywords", "types");
        PoolColorArray rust_colors_array = cast_to<EditorFile>(this->get_parent())->load_config("user://syntax.cfg", "Rust", keys);
        rust_colors["use_declaration"] = rust_colors_array[0];
        rust_colors["identifier"] = rust_colors_array[1];
        rust_colors["type_identifier"] = Color(0.24, 0.84, 0.24, 1.0);
        rust_colors["line_comment"] = rust_colors_array[2];
        rust_colors["meta_arguments"] = rust_colors_array[4];
        rust_colors["boolean_literal"] = Color(0.64, 0.4, 0.0, 1.0);
        rust_colors["number_literal"] = Color(0.64, 0.4, 0.0, 1.0);
        rust_colors["string_literal"] =Color(0.64, 0.4, 0.0, 1.0);

        for (int i = 0; i < rust_keywords.size(); i++)
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(rust_keywords[i], rust_colors_array[3]);
        }
        node_array = parse_text(((TextEdit *)get_node("Container/CodeEditor"))->get_text(), lang);
        setup_rust_colors(node_array);
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
        Color color = Color(1.0, 1.0, 1.0, 1.0);
        if (cpp_colors.has(current_node[0]))
        {
            color = cpp_colors[current_node[0]];
        }
        String node_name = current_node[0];
        if (node_name.substr(0, 7) == String("preproc"))
        {
            color = cpp_colors["preproc"];
            keyword = ((TextEdit *)get_node("Container/CodeEditor"))->get_text().substr(start + 1, len - 1);
            ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region(keyword, "", color, false);
        }
        else if (node_name == String("comment"))
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region(keyword, "", color, false);
        }
        else if (node_name == String("identifier") || node_name == String("field_identifier"))
        {
            if (!cpp_keywords.has(keyword))
            {
                color = cpp_colors["identifier"];
                ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(keyword, color);
            }
        }
        else if (node_name == String("namespace_identifier"))
        {
            if (!cpp_keywords.has(keyword))
            {
                ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(keyword, color);
            }
        }
        else if (node_name == String("system_lib_string"))
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region(keyword, "", color, false);
        }
        else if (node_name == String("number_literal"))
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(keyword, color);
        }
        if (current_node[3] != String(""))
        {
            this->setup_cpp_colors(current_node[3]);
        }
    }
    ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region("\"", "\"", cpp_colors["string_literal"], false);
}

void CodeEditor::setup_rust_colors(Array node_array)
{
    for (int i = 0; i < node_array.size(); i++)
    {
        Array current_node = node_array[i];
        int start = int(current_node[1]);
        int end = int(current_node[2]);
        int len = int(end - start);
        String keyword = ((TextEdit *)get_node("Container/CodeEditor"))->get_text().substr(start, len);
        Color color = Color(1.0, 1.0, 1.0, 1.0);
        if (rust_colors.has(current_node[0]))
        {
            color = rust_colors[current_node[0]];
        }
        String node_name = current_node[0];
        if (node_name == String("use_declaration"))
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region(keyword, "", color, false);
        }
        else if (node_name == String("meta_arguments"))
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region(keyword, "", color, false);
        }
        else if (node_name == String("line_comment") || node_name == String("block_comment"))
        {
            color = rust_colors["line_comment"];
            ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region(keyword, "", color, false);
        }
        else if (node_name == String("meta_arguments"))
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(keyword, color);
        }
        else if (node_name == String("type_identifier"))
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(keyword, color);
        }
        else if (node_name == String("boolean_literal"))
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(keyword, color);
        }
        else if (node_name == String("integer_literal") || node_name == String("float_literal"))
        {
            color = rust_colors["number_literal"];
            ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(keyword, color);
        }
        else if (node_name == String("identifier") || node_name == String("field_identifier"))
        {
            if (!rust_keywords.has(keyword))
            {
                color = rust_colors["identifier"];
                ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(keyword, color);
            }
        }
        if (current_node[3] != String(""))
        {
            this->setup_rust_colors(current_node[3]);
        }
    }
    ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region("\'", "\'", rust_colors["string_literal"], false);
    ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region("\"", "\"", rust_colors["string_literal"], false);
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
            node_array = parse_text(((TextEdit *)get_node("Container/CodeEditor"))->get_text(), this->language);
            if (this->language == "cpp")
            {
                setup_cpp_colors(node_array);
            }
            else if (this->language == "rust")
            {
                setup_rust_colors(node_array);
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
    register_method((char *)"setup_rust_colors", &CodeEditor::setup_rust_colors);
    register_method((char *)"get_content", &CodeEditor::get_content);
    register_method((char *)"save_contents", &CodeEditor::save_contents);
    register_method((char *)"get_text_changed", &CodeEditor::get_text_changed);
    register_method((char *)"set_custom_font", &CodeEditor::set_custom_font);
    register_method((char *)"set_font_size", &CodeEditor::set_font_size);
    register_method((char *)"set_custom_theme", &CodeEditor::set_custom_theme);

    register_method((char *)"_on_CodeEditor_text_changed", &CodeEditor::_on_CodeEditor_text_changed);
    register_method((char *)"_on_CodeEditor_gui_input", &CodeEditor::_on_CodeEditor_gui_input);
}

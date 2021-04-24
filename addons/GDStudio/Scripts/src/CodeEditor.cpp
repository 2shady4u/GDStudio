#include <Godot.hpp>
#include <Control.hpp>
#include <Array.hpp>
#include <TextEdit.hpp>
#include <Color.hpp>
#include <InputEvent.hpp>
#include <InputEventMouseButton.hpp>
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
#include <OS.hpp>
#include <ItemList.hpp>
#include <thread>
#include <future>

#include "CodeEditor.hpp"
#include "FileManager.hpp"
using namespace godot;

extern "C" TSLanguage *tree_sitter_cpp();
extern "C" TSLanguage *tree_sitter_rust();

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

void CodeEditor::parse_text(String language)
{
    TSParser *parser = ts_parser_new();
    if (language == "cpp")
    {
        ts_parser_set_language(parser, tree_sitter_cpp());
    }
    else if (language == "rust")
    {
        ts_parser_set_language(parser, tree_sitter_rust());
    }
    const char *source_code = ((TextEdit *)get_node("Container/CodeEditor"))->get_text().utf8().get_data();
    tree = ts_parser_parse_string(
        parser,
        NULL,
        source_code,
        strlen(source_code));
    TSNode node_array = ts_tree_root_node(tree);

    if (language == "cpp")
    {
        setup_cpp_colors(node_array);
    }
    else if (language == "rust")
    {
        setup_rust_colors(node_array);
    }

    ts_tree_delete(tree);
    ts_parser_delete(parser);
}

void CodeEditor::set_initial_content(String content)
{
    line_space = ((TextEdit *)get_node("Container/CodeEditor"))->get("custom_constants/line_spacing");
    ((TextEdit *)get_node("Container/CodeEditor"))->set_text(content);
    this->current_content = content;
    ((TextEdit *)get_node("Container/CodeEditor"))->cursor_set_line(0);
    ((TextEdit *)get_node("Container/CodeEditor"))->cursor_set_column(1);
    PoolStringArray keys = Array::make("use_treesitter");
    Array settings = cast_to<EditorFile>(this->get_parent())->load_config("user://editor.cfg", "Editor", keys);
    this->use_tree_sitter = settings[0];
    editor_height = this->get_size()[1];
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
        autocomplete.append_array(cpp_keywords);
        if (this->use_tree_sitter)
        {
            parse_text(lang);
        }
        this->language = "cpp";
    }
    else if (lang == "rust")
    {
        PoolStringArray keys = Array::make("identifiers", "types", "comments", "strings", "numbers", "keywords");
        PoolColorArray rust_colors_array = cast_to<EditorFile>(this->get_parent())->load_config("user://syntax.cfg", "Rust", keys);
        rust_colors["use_declaration"] = rust_colors_array[0];
        rust_colors["identifier"] = rust_colors_array[0];
        rust_colors["type_identifier"] = rust_colors_array[1];
        rust_colors["line_comment"] = rust_colors_array[2];
        rust_colors["meta_arguments"] = rust_colors_array[5];
        rust_colors["boolean_literal"] = rust_colors_array[5];
        rust_colors["number_literal"] = rust_colors_array[4];
        rust_colors["string_literal"] = rust_colors_array[3];

        for (int i = 0; i < rust_keywords.size(); i++)
        {
            ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(rust_keywords[i], rust_colors_array[5]);
        }
        autocomplete.append_array(rust_keywords);
        if (this->use_tree_sitter)
        {
            parse_text(lang);
        }
        this->language = "rust";
    }
}

void CodeEditor::setup_cpp_colors(TSNode root_node)
{
    for (int i = 0; i < ts_node_named_child_count(root_node); i++)
    {
        TSNode child_node = ts_node_named_child(root_node, i);
        String node_name = String(ts_node_type(child_node));
        int start = int(ts_node_start_byte(child_node));
        int end = int(ts_node_end_byte(child_node));
        int len = int(end - start);
        String keyword = ((TextEdit *)get_node("Container/CodeEditor"))->get_text().substr(start, len);
        Color color = Color(1.0, 1.0, 1.0, 1.0);
        if (cpp_colors.has(node_name))
        {
            color = cpp_colors[node_name];
        }
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
        if (ts_node_child_count(child_node) > 0)
        {
            this->setup_cpp_colors(child_node);
        }
    }
    ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region("\"", "\"", cpp_colors["string_literal"], false);
}

void CodeEditor::setup_rust_colors(TSNode root_node)
{
    for (int i = 0; i < ts_node_named_child_count(root_node); i++)
    {
        TSNode child_node = ts_node_named_child(root_node, i);
        String node_name = String(ts_node_type(child_node));
        int start = int(ts_node_start_byte(child_node));
        int end = int(ts_node_end_byte(child_node));
        int len = int(end - start);
        String keyword = ((TextEdit *)get_node("Container/CodeEditor"))->get_text().substr(start, len);
        Color color = Color(1.0, 1.0, 1.0, 1.0);
        if (rust_colors.has(node_name))
        {
            color = rust_colors[node_name];
        }
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
        if (ts_node_child_count(child_node) > 0)
        {
            this->setup_rust_colors(child_node);
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

String CodeEditor::get_current_word(int64_t column, int64_t line)
{
    String word = "";
    int64_t start = column;
    String line_text = ((TextEdit *)get_node("Container/CodeEditor"))->get_line(line);
    while (!line_text.empty() && line_text[start] != ' ' && line_text[start] != '\n' && line_text[start] != '\t')
    {
        start -= 1;
    }
    ((TextEdit *)get_node("Container/CodeEditor"))->select(line, start, line, column);
    word = ((TextEdit *)get_node("Container/CodeEditor"))->get_selection_text();
    word = word.strip_edges(true);
    ((TextEdit *)get_node("Container/CodeEditor"))->select(line, column, line, column);
    return word;
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
    font_size = cast_to<DynamicFont>(*((TextEdit *)get_node("Container/CodeEditor"))->get_font("font"))->get_height();
}

void CodeEditor::set_custom_theme(String path)
{
    Ref<Theme> theme = ResourceLoader::get_singleton()->load(path);
    cast_to<Control>(this->get_parent()->get_parent())->set_theme(theme);
}
int CodeEditor::select_current_word()
{
    int64_t column = ((TextEdit *)get_node("Container/CodeEditor"))->cursor_get_column();
    int64_t line = ((TextEdit *)get_node("Container/CodeEditor"))->cursor_get_line();
    int64_t start = column;
    String line_text = ((TextEdit *)get_node("Container/CodeEditor"))->get_line(line);
    while (!line_text.empty() && line_text[start] != ' ' && line_text[start] != '\n' && line_text[start] != '\t')
    {
        start -= 1;
    }
    ((TextEdit *)get_node("Container/CodeEditor"))->select(line, start, line, column);
    return column - start;
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
    InputEventMouseButton *mouse_key = cast_to<InputEventMouseButton>(event);
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
                if (this->use_tree_sitter)
                {
                    parse_text(this->language);
                }

                break;
            }
            ((TextEdit *)get_node("Container/CodeEditor"))->select(line, column, line, column);
            ((TextEdit *)get_node("Container/CodeEditor"))->cursor_set_column(column);
            String word = get_current_word(column, line);
            if (word != "" && word != " " && !word.empty())
            {
                if (event_key->get_scancode() != GlobalConstants::KEY_DOWN && event_key->get_scancode() != GlobalConstants::KEY_UP && event_key->get_scancode() != GlobalConstants::KEY_LEFT && event_key->get_scancode() != GlobalConstants::KEY_RIGHT)
                {
                    int v_scroll = ((TextEdit *)get_node("Container/CodeEditor"))->get_v_scroll();
                    ((ItemList *)get_node(NodePath("Container/Autocomplete")))->clear();
                    for (int i = 0; i < autocomplete.size(); i++)
                    {
                        if (autocomplete[i].find(word, 0) == 0)
                        {
                            //((ItemList *)get_node(NodePath("Container/Autocomplete")))->add_item("Keyword", nullptr, false);
                            ((ItemList *)get_node(NodePath("Container/Autocomplete")))->add_item(autocomplete[i]);
                            //((ItemList *)get_node(NodePath("Container/Autocomplete")))->set_item_tooltip(i, "Keyword");
                        }
                    }
                    if (((ItemList *)get_node(NodePath("Container/Autocomplete")))->get_size()[1] > 196)
                    {
                        ((ItemList *)get_node(NodePath("Container/Autocomplete")))->set_size(Vector2(256, 196));
                    }
                    int v_size = ((ItemList *)get_node(NodePath("Container/Autocomplete")))->get_size()[1];
                    int x = column * (font_size + line_space);
                    int y = (line + 1 - v_scroll) * (font_size + line_space) + 8;
                    if (y + v_size >= editor_height)
                    {
                        y -= v_size + (font_size + line_space);
                    }
                    ((ItemList *)get_node(NodePath("Container/Autocomplete")))->set_position(Vector2(x, y));

                    ((ItemList *)get_node(NodePath("Container/Autocomplete")))->sort_items_by_text();
                    ((ItemList *)get_node(NodePath("Container/Autocomplete")))->select(0);
                    if (((ItemList *)get_node(NodePath("Container/Autocomplete")))->get_item_count() == 1 && word == ((ItemList *)get_node(NodePath("Container/Autocomplete")))->get_item_text(0))
                    {
                        ((ItemList *)get_node(NodePath("Container/Autocomplete")))->hide();
                    }
                    else if (((ItemList *)get_node(NodePath("Container/Autocomplete")))->get_item_count() > 0)
                    {
                        ((ItemList *)get_node(NodePath("Container/Autocomplete")))->show();
                    }
                    else
                    {
                        ((ItemList *)get_node(NodePath("Container/Autocomplete")))->hide();
                    }
                }
                else
                {
                    ((ItemList *)get_node(NodePath("Container/Autocomplete")))->hide();
                }
            }
            else
            {
                ((ItemList *)get_node(NodePath("Container/Autocomplete")))->hide();
            }
        }
    }
    if (mouse_key)
    {
        if (mouse_key->is_pressed())
        {
            ((ItemList *)get_node(NodePath("Container/Autocomplete")))->hide();
        }
    }
}

void CodeEditor::_on_CodeEditor_symbol_lookup(String symbol, int row, int column)
{
    Godot::print(symbol);
}

void CodeEditor::_on_Autocomplete_item_activated(int index)
{
    int size = this->select_current_word();
    int64_t column = ((TextEdit *)get_node("Container/CodeEditor"))->cursor_get_column();
    int64_t line = ((TextEdit *)get_node("Container/CodeEditor"))->cursor_get_line();
    String sub = ((ItemList *)get_node(NodePath("Container/Autocomplete")))->get_item_text(index);
    ((TextEdit *)get_node("Container/CodeEditor"))->insert_text_at_cursor(sub);
    ((ItemList *)get_node(NodePath("Container/Autocomplete")))->hide();
    ((TextEdit *)get_node("Container/CodeEditor"))->grab_focus();
    ((TextEdit *)get_node("Container/CodeEditor"))->cursor_set_line(line);
    ((TextEdit *)get_node("Container/CodeEditor"))->cursor_set_column(column + size);
}

void CodeEditor::_register_methods()
{
    register_method((char *)"_init", &CodeEditor::_init);
    register_method((char *)"set_initial_content", &CodeEditor::set_initial_content);
    //register_method((char *)"parse_text", &CodeEditor::parse_text);
    register_method((char *)"setup_language", &CodeEditor::setup_language);
    //register_method((char *)"setup_cpp_colors", &CodeEditor::setup_cpp_colors);
    //register_method((char *)"setup_rust_colors", &CodeEditor::setup_rust_colors);
    register_method((char *)"get_content", &CodeEditor::get_content);
    register_method((char *)"get_current_word", &CodeEditor::get_current_word);
    register_method((char *)"save_contents", &CodeEditor::save_contents);
    register_method((char *)"get_text_changed", &CodeEditor::get_text_changed);
    register_method((char *)"set_custom_font", &CodeEditor::set_custom_font);
    register_method((char *)"set_font_size", &CodeEditor::set_font_size);
    register_method((char *)"set_custom_theme", &CodeEditor::set_custom_theme);
    register_method((char *)"select_current_word", &CodeEditor::select_current_word);

    register_method((char *)"_on_CodeEditor_text_changed", &CodeEditor::_on_CodeEditor_text_changed);
    register_method((char *)"_on_CodeEditor_gui_input", &CodeEditor::_on_CodeEditor_gui_input);
    register_method((char *)"_on_Autocomplete_item_activated", &CodeEditor::_on_Autocomplete_item_activated);
}

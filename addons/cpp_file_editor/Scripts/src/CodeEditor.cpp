#include <Godot.hpp>
#include <Control.hpp>
#include <Array.hpp>
#include <TextEdit.hpp>
#include <Color.hpp>

#include "CodeEditor.hpp"
using namespace godot;

CodeEditor::CodeEditor()
{
}

CodeEditor::~CodeEditor()
{
}

void CodeEditor::_init()
{
    Godot::print(preprocessor[1]);
}

void CodeEditor::_ready()
{
    ((TextEdit *)get_node("Container/CodeEditor"))->get_meta("custom_colors");
}

void CodeEditor::set_initial_content(String content) {

    ((TextEdit *)get_node("Container/CodeEditor"))->set_text(content);
    this->current_content = content;
    this->setup_syntax();
}

void CodeEditor::setup_syntax()
{
    ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region("\"", "\"",Color(128, 64, 0,255),true);
    ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region("//", "",Color(0, 192, 64,255),true);
    for (int i = 0; i < this->preprocessor.size(); i++) {
        ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region(i, "",Color(0, 128, 64,255),true);
    }
    for (int i = 0; i < this->keywords.size(); i++) {
        ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region(i, "",Color(128,0,255,255));
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
    if ((TextEdit *)get_node("Container/CodeEditor")).text == this->current_content
    {
        this->text_changed = false;
    }else{
        this->text_changed = true;
    }
    this->line_number = ((TextEdit *)get_node("Container/CodeEditor"))->get_line_count();
}

void CodeEditor::_register_methods()
{
    register_method((char *)"_init", &CodeEditor::_init);
}

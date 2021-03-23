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
    ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region("\"", "\"",Color(0.5, 0.25, 0,1),true);
    ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region("//", "",Color(0, 0.5, 0.25,1),true);
    ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region("/*", "*/",Color(0, 0.5, 0.25,1));
    for (int j = 0; j < this->preprocessor.size(); j++) {
        ((TextEdit *)get_node("Container/CodeEditor"))->add_color_region(this->preprocessor[j], "",Color(0.6, 0, 0.8, 1), true);
    }
    for (int i = 0; i < this->keywords.size(); i++) {
        ((TextEdit *)get_node("Container/CodeEditor"))->add_keyword_color(this->keywords[i], Color(0, 0.8, 0.75,1));
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
    if (((TextEdit *)get_node("Container/CodeEditor"))->get_text() == this->current_content) {
        this->text_changed = false;
    }else{
        this->text_changed = true;
    }
}

bool CodeEditor::get_text_changed()
{
    return this->text_changed;
}

void CodeEditor::_register_methods()
{
    register_method((char *)"_init", &CodeEditor::_init);
    register_method((char *)"set_initial_content", &CodeEditor::set_initial_content);
    register_method((char *)"setup_syntax", &CodeEditor::setup_syntax);
    register_method((char *)"get_content", &CodeEditor::get_content);
    register_method((char *)"save_contents", &CodeEditor::save_contents);
    register_method((char *)"get_text_changed", &CodeEditor::get_text_changed);
    register_method((char *)"_on_CodeEditor_text_changed", &CodeEditor::_on_CodeEditor_text_changed);
}

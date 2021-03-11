#include <Godot.hpp>
#include <Control.hpp>
#include <Array.hpp>

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
    Godot::print(preprocessor[1]);
}

void CodeEditor::_register_methods()
{
    register_method((char *)"_init", &CodeEditor::_init);
}

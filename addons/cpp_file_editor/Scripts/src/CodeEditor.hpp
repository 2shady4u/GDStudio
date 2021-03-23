#include <Godot.hpp>
#include <Control.hpp>
#include <String.hpp>
#include <Array.hpp>
#include <Variant.hpp>

using namespace godot;

#ifndef CodeEditor_HEADER
#define CodeEditor_HEADER
class CodeEditor : public Control
{
    GODOT_CLASS(CodeEditor, Control);

public:
    CodeEditor();
    ~CodeEditor();
    void _init();
    void _ready();
    void set_initial_content(String);
    void setup_syntax();
    String get_content();
    void save_contents();
    void _on_CodeEditor_text_changed();
    void _input(const Ref<InputEvent>);

    static void _register_methods();

private:
    String search_path = "E:/codeblocks-20.03mingw-nosetup/MinGW/include";
    int line_index = 0;
    int line_number = 0;
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
    String error_text = "";
    String current_content = "";
    String file_name = "";
    bool text_changed = false;
    enum variable_types
    {
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
        prepr,
        semicolon,
        comment,
        illegal,
    };
};

#endif
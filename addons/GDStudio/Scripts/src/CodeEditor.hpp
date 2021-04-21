#include <Godot.hpp>
#include <Control.hpp>
#include <String.hpp>
#include <Array.hpp>
#include <Variant.hpp>
#include <InputEvent.hpp>
#include <TreeItem.hpp>

#include <tree_sitter/api.h>
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
    void parse_text(String);
    void setup_language(String);
    void setup_cpp_colors(TSNode);
    void setup_rust_colors(TSNode);
    String get_content();
    String get_language();
    void save_contents();
    void set_custom_font(String);
    void set_font_size(int);
    void set_custom_theme(String);

    void _on_CodeEditor_text_changed();
    void _on_CodeEditor_symbol_lookup(String, int, int);

    bool get_text_changed();
    void _on_CodeEditor_gui_input(InputEvent *);

    static void _register_methods();

private:
    TSTree *tree;
    Dictionary cpp_colors;
    Dictionary rust_colors;
    String language = "";
    String error_text = "";
    String current_content = "";
    String file_name = "";
    bool use_tree_sitter = true;
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
    Array cpp_keywords = Array::make("auto", "short", "struct", "unsigned",
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
    Array rust_keywords = Array::make("alignof", "as", "be", "box", "break", "const", "continue", "crate",
                                 "do", "else", "enum", "extern", "false", "fn", "for", "if", "impl", "in", "let", "loop",
                                 "match", "mod", "mut", "offsetof", "once", "priv", "proc", "pub", "pure", "ref", "return", "self",
                                 "sizeof", "static", "struct", "super", "trait", "true", "type", "typeof", "unsafe", "unsized",
                                 "use", "virtual", "while", "yield", "bool", "char", "f32", "f64", "i8", "i16", "i32", "i64",
                                 "str", "u8", "u16", "u32", "u64", "Self", "self");
    Array operators = Array::make("+", "-", "*", "/", "=", "+=", "-=", "*=", "/=", "==", "++", "--",
                                  "!", "!=", "%", "%=", "<", ">", "<=", ">=", "&&", "||", "&", "|", "^", "~", "<<", ">>");
};

#endif
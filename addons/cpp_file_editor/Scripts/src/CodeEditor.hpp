#include <Godot.hpp>
#include <Control.hpp>
#include <String.hpp>
#include <Array.hpp>
#include <Variant.hpp>
#include <InputEvent.hpp>
#include <TreeItem.hpp>

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
    void setup_language(String);
    String get_content();
    String get_language();
    void save_contents();
    void set_custom_font(String);
    void set_font_size(int);
    void set_custom_theme(String);

    void _on_CodeEditor_text_changed();
    void _on_CodeEditor_symbol_lookup(String, int, int);

    bool get_text_changed();
    void _on_CodeEditor_gui_input(InputEvent*);

    static void _register_methods();

private:
    String language = "";
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
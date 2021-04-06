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
    void setup_syntax();
    void setup_language(String);
    String get_content();
    void save_contents();
    void edit_log(String);
    String get_build_platform_cpp();
    void set_custom_font(String);
    void set_font_size(int);
    void set_custom_theme(String);
    bool get_release_flag();
    void list_directories(String);
    void list_subdirectories(String, TreeItem*);

    void _on_CodeEditor_text_changed();
    void _on_CodeEditor_symbol_lookup(String, int, int);
    void _on_ExecuteCustomCommandButton_pressed();

    bool get_text_changed();
    void _on_CodeEditor_gui_input(InputEvent*);
    void _on_Build_pressed();
    void _on_Clean_pressed();

    static void _register_methods();

private:
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
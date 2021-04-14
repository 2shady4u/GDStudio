#include <Godot.hpp>
#include <String.hpp>

#include <tree_sitter/api.h>

using namespace godot;

TSLanguage *tree_sitter_cpp();
int test_parse(String text)
{
    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_cpp());
    const char *source_code = "double i = 0.2;";
    TSTree *tree = ts_parser_parse_string(
        parser,
        NULL,
        source_code,
        strlen(source_code));
    TSNode root_node = ts_tree_root_node(tree);
    char *string = ts_node_string(root_node);
    Godot::print(string);
    free(string);
    ts_tree_delete(tree);
    ts_parser_delete(parser);
    return 0;
}
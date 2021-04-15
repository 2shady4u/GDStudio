#include <Godot.hpp>
#include <String.hpp>
#include <Array.hpp>

#include <tree_sitter/api.h>
using namespace godot;

extern "C" TSLanguage *tree_sitter_cpp();

Array get_child_array(TSNode root_node)
{
    Array array;
    for (int i = 0; i < ts_node_named_child_count(root_node); i++)
    {
        Array node;
        TSNode child_node = ts_node_named_child(root_node, i);
        const char *name = ts_node_type(child_node);
        int start = ts_node_start_byte(child_node);
        int end = ts_node_end_byte(child_node);
        node.append(name);
        node.append(start);
        node.append(end);
        if (ts_node_child_count(child_node) > 0)
        {
            node.append(get_child_array(child_node));
        }
        array.append(node);
    }
    return array;
}

Array test_parse(String text)
{
    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_cpp());
    const char *source_code = text.utf8().get_data();
    TSTree *tree = ts_parser_parse_string(
        parser,
        NULL,
        source_code,
        strlen(source_code));
    TSNode root_node = ts_tree_root_node(tree);
    Array node_array = get_child_array(root_node);
    ts_tree_delete(tree);
    ts_parser_delete(parser);
    return node_array;
}
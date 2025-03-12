#ifndef AST_FWD_HPP
#define AST_FWD_HPP

#include <types.hpp>

namespace supdef
{
    namespace ast
    {
        class node;

        class directive_node;
        class expression_node;
        class block_node;
        class text_node;

        using shared_node = ::supdef::shared_ptr<node>;

        using shared_directive = ::supdef::shared_ptr<directive_node>;
        using shared_expression = ::supdef::shared_ptr<expression_node>;
        using shared_block = ::supdef::shared_ptr<block_node>;

        using shared_text = ::supdef::shared_ptr<text_node>;

        class import_node;
        class supdef_node;
        class runnable_node;
        class pragma_node;
        class dump_node;
        class embed_node;
        class set_node;
        class unset_node;

        class for_node;
        class foreach_node;
        class foreachi_node;
        class conditional_node;

        class builtin_node;
        class varsubst_node;
        class macrocall_node;
        class unaryop_node;
        class binaryop_node;
        class string_node;
        class integer_node;
        class floating_node;
        class boolean_node;
        class list_node;
    }
}

#endif

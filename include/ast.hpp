#ifndef AST_HPP
#define AST_HPP

#include <types.hpp>
#include <tokenizer.hpp>
#include <version.hpp>
#include <parser.hpp>

#include <bits/stdc++.h>

namespace supdef
{
    namespace ast
    {
        class pragma_node;
        class set_node;
        class unset_node;
        class if_node;
        class elseif_node;
        class else_node;
        class for_node;
        class foreach_node;
        class foreachi_node;
        class builtin_node;
        class embed_node;
        class dump_node;
        class varsubst_node;
        class macrocall_node;
        class unaryop_node;
        class binaryop_node;
        class text_node;
        
        using node = shared_ptr<
            std::variant<
                pragma_node,
                set_node,
                unset_node,
                if_node,
                elseif_node,
                else_node,
                for_node,
                foreach_node,
                foreachi_node,
                builtin_node,
                embed_node,
                dump_node,
                varsubst_node,
                macrocall_node,
                unaryop_node,
                binaryop_node,
                text_node
            >
        >;

        class node_base
        {
        public:
            virtual ~node_base() = default;
        };

        // @pragma ...
        class pragma_node
            : public virtual node_base
        {
        public:
            enum class kind
            {
                supdef,
                runnable,
                runnable_lang,
                other,
                unknown
            };

            pragma_node(kind k, token p, std::vector<node> v)
                : m_lang(std::nullopt)
                , m_prag(p)
                , m_val(v)
                , m_kind(k)
            {
                assert(m_kind != kind::unknown);
                assert(m_kind != kind::runnable_lang);
            }

            pragma_node(kind k, std::vector<token> l, token p, std::vector<node> v)
                : m_lang(l)
                , m_prag(p)
                , m_val(v)
                , m_kind(k)
            {
                assert(m_kind == kind::runnable_lang);
            }

            pragma_node(std::vector<token> l, token p, std::vector<node> v)
                : m_lang(l)
                , m_prag(p)
                , m_val(v)
                , m_kind(kind::runnable_lang)
            {
            }
        private:
            std::optional<std::vector<token>> m_lang;
            token m_prag;
            std::vector<node> m_val;
            kind m_kind;
        };
    }
}

#endif

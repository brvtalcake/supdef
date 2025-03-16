#ifndef AST_HPP
#define AST_HPP

#include <types.hpp>
#include <tokenizer.hpp>
#include <version.hpp>
#include <parser.hpp>
#include <printer.hpp>

#include <bits/stdc++.h>
#include <cstdint>

namespace supdef
{
    namespace ast
    {
        class node
        {
        public:
            enum class kind
                : uint64_t
            {
#pragma push_macro("IS")
#pragma push_macro("INDEX")
#undef  IS
#undef  INDEX
#define IS(k)  ((k) << 4)
#define INDEX(underlying, k) (UINT64_C(1) << ((sizeof(underlying) * 8) - (k) - 1))
                unknown = UINT64_C(0),

                directive  = UINT64_C(1) << 0,
                expression = UINT64_C(1) << 1,
                block      = UINT64_C(1) << 2,

                import   = INDEX(uint64_t, 0) | IS(directive),
                supdef   = INDEX(uint64_t, 1) | IS(directive),
                runnable = INDEX(uint64_t, 2) | IS(directive),
                pragma   = INDEX(uint64_t, 3) | IS(directive),
                dump     = INDEX(uint64_t, 4) | IS(directive),
                embed    = INDEX(uint64_t, 5) | IS(directive),
                set      = INDEX(uint64_t, 6) | IS(directive),
                unset    = INDEX(uint64_t, 7) | IS(directive),

                for_        = INDEX(uint64_t,  8) | IS(block),
                foreach     = INDEX(uint64_t,  9) | IS(block),
                foreachi    = INDEX(uint64_t, 10) | IS(block),
                conditional = INDEX(uint64_t, 11) | IS(block),


                builtin   = INDEX(uint64_t, 12) | IS(expression),
                varsubst  = INDEX(uint64_t, 13) | IS(expression),
                macrocall = INDEX(uint64_t, 14) | IS(expression),
                unaryop   = INDEX(uint64_t, 15) | IS(expression),
                binaryop  = INDEX(uint64_t, 16) | IS(expression),
                string    = INDEX(uint64_t, 17) | IS(expression), ///< can be a "..." string or a '...' string
                integer   = INDEX(uint64_t, 18) | IS(expression),
                floating  = INDEX(uint64_t, 19) | IS(expression),
                boolean   = INDEX(uint64_t, 20) | IS(expression),
                list      = INDEX(uint64_t, 21) | IS(expression),

                // don't include maps for now
                /* map       = INDEX(uint64_t, 22) | IS(expression), */

                text = INDEX(uint64_t, 23) ///< anything not related to supdef, i.e.\
                                                is directly output to the output file
            };

        public:
            node() = default;
            virtual ~node() = default;

            node(::supdef::token_loc&& loc)
                : m_loc(std::move(loc))
            {
            }
            node(const ::supdef::token_loc& loc)
                : m_loc(loc)
            {
            }

            virtual kind node_kind() const
            {
                return kind::unknown;
            }

            bool operator==(kind k) const
            {
                return this->is(k);
            }

            kind operator&(kind k) const;
            kind operator|(kind k) const;
            kind operator^(kind k) const;
            kind operator~() const;

            template <typename Self>
            bool is(this Self&& self, kind k);

            const ::supdef::token_loc& location() const &
            {
                return m_loc;
            }
            ::supdef::token_loc&& location() &&
            {
                return std::move(m_loc);
            }
            

        private:
            ::supdef::token_loc m_loc;
        };
        class directive_node;
        class expression_node;
        class block_node;
        class text_node;
        
        using shared_node = ::supdef::shared_ptr<node>;
        using shared_directive = ::supdef::shared_ptr<directive_node>;
        using shared_expression = ::supdef::shared_ptr<expression_node>;
        using shared_block = ::supdef::shared_ptr<block_node>;
        using shared_text = ::supdef::shared_ptr<text_node>;
        
        template <typename LhsT, typename RhsT, typename EnumT>
        concept valid_enum_class_op_combination_lhs_is_enum_class = std::same_as<LhsT, EnumT> &&
            (std::same_as<RhsT, EnumT> || std::same_as<RhsT, std::underlying_type_t<EnumT>> || std::convertible_to<RhsT, std::underlying_type_t<EnumT>>);
        
        template <typename LhsT, typename RhsT, typename EnumT>
        concept valid_enum_class_op_combination_rhs_is_enum_class = std::same_as<RhsT, EnumT> &&
            (std::same_as<LhsT, EnumT> || std::same_as<LhsT, std::underlying_type_t<EnumT>> || std::convertible_to<LhsT, std::underlying_type_t<EnumT>>);
        
        template <typename LhsT, typename RhsT, typename EnumT>
        concept valid_enum_class_op_combination = valid_enum_class_op_combination_lhs_is_enum_class<LhsT, RhsT, EnumT> ||
                                                  valid_enum_class_op_combination_rhs_is_enum_class<LhsT, RhsT, EnumT>;
        
        template <typename LhsT, typename RhsT, typename EnumT>
        concept enum_class_op_is_noexcept = std::is_nothrow_convertible_v<LhsT, std::underlying_type_t<EnumT>> &&
                                            std::is_nothrow_convertible_v<RhsT, std::underlying_type_t<EnumT>>;
        
        // operators for enum class kind
        template <typename LhsT, typename RhsT>
            requires valid_enum_class_op_combination<LhsT, RhsT, node::kind>
        inline node::kind operator&(LhsT lhs, RhsT rhs) noexcept(enum_class_op_is_noexcept<LhsT, RhsT, node::kind>)
        {
            return static_cast<node::kind>(std::to_underlying(lhs) & std::to_underlying(rhs));
        }
        
        template <typename LhsT, typename RhsT>
            requires valid_enum_class_op_combination<LhsT, RhsT, node::kind>
        inline node::kind operator|(LhsT lhs, RhsT rhs) noexcept(enum_class_op_is_noexcept<LhsT, RhsT, node::kind>)
        {
            return static_cast<node::kind>(std::to_underlying(lhs) | std::to_underlying(rhs));
        }
        
        template <typename LhsT, typename RhsT>
            requires valid_enum_class_op_combination<LhsT, RhsT, node::kind>
        inline node::kind operator^(LhsT lhs, RhsT rhs) noexcept(enum_class_op_is_noexcept<LhsT, RhsT, node::kind>)
        {
            return static_cast<node::kind>(std::to_underlying(lhs) ^ std::to_underlying(rhs));
        }

        inline node::kind operator~(node::kind k) noexcept
        {
            return static_cast<node::kind>(~std::to_underlying(k));
        }
        
        inline node::kind operator<<(node::kind k, std::make_unsigned_t<std::underlying_type_t<node::kind>> n) noexcept
        {
            return static_cast<node::kind>(std::to_underlying(k) << n);
        }
        
        inline node::kind operator>>(node::kind k, std::make_unsigned_t<std::underlying_type_t<node::kind>> n) noexcept
        {
            return static_cast<node::kind>(std::to_underlying(k) >> n);
        }
        
        template <typename LhsT, typename RhsT>
            requires valid_enum_class_op_combination<LhsT, RhsT, node::kind>
        inline node::kind& operator&=(LhsT& lhs, RhsT rhs) noexcept(enum_class_op_is_noexcept<LhsT, RhsT, node::kind>)
        {
            return lhs = lhs & rhs;
        }
        
        template <typename LhsT, typename RhsT>
            requires valid_enum_class_op_combination<LhsT, RhsT, node::kind>
        inline node::kind& operator|=(LhsT& lhs, RhsT rhs) noexcept(enum_class_op_is_noexcept<LhsT, RhsT, node::kind>)
        {
            return lhs = lhs | rhs;
        }
        
        template <typename LhsT, typename RhsT>
            requires valid_enum_class_op_combination<LhsT, RhsT, node::kind>
        inline node::kind& operator^=(LhsT& lhs, RhsT rhs) noexcept(enum_class_op_is_noexcept<LhsT, RhsT, node::kind>)
        {
            return lhs = lhs ^ rhs;
        }
        
        inline node::kind& operator<<=(node::kind& lhs, std::make_unsigned_t<std::underlying_type_t<node::kind>> n) noexcept
        {
            return lhs = lhs << n;
        }
        
        inline node::kind& operator>>=(node::kind& lhs, std::make_unsigned_t<std::underlying_type_t<node::kind>> n) noexcept
        {
            return lhs = lhs >> n;
        }
        
        node::kind node::operator&(node::kind k) const
        {
            return this->node_kind() & k;
        }
        node::kind node::operator|(node::kind k) const
        {
            return this->node_kind() | k;
        }
        node::kind node::operator^(node::kind k) const
        {
            return this->node_kind() ^ k;
        }
        node::kind node::operator~() const
        {
            return ~this->node_kind();
        }

        template <typename Self>
        bool node::is(this Self&& self, node::kind k)
        {
            switch (k)
            {
            case node::kind::directive:
                [[__fallthrough__]];
            case node::kind::expression:
                [[__fallthrough__]];
            case node::kind::block: {
                    typedef std::remove_cvref_t<Self> self_type;
                    if constexpr (std::is_final_v<self_type>)
                        return static_cast<bool>(std::forward<Self>(self).node_kind() & IS(k));
                }
                [[__fallthrough__]];
            default:
                return std::forward<Self>(self).node_kind() == k;
            }
        }

#pragma pop_macro("IS")
#pragma pop_macro("INDEX")

        namespace detail
        {
            namespace test
            {
                consteval bool all_node_kinds_have_unique_values()
                {
                    std::array values = magic_enum::enum_values<node::kind>();
                    stdranges::sort(values);
                    return stdranges::adjacent_find(values) == stdranges::end(values);
                }
            }
        }
    }
}

static_assert(supdef::ast::detail::test::all_node_kinds_have_unique_values());

#include <impl/ast-nodes/bases.tpp>

#include <impl/ast-nodes/import.tpp>
#include <impl/ast-nodes/supdef.tpp>
#include <impl/ast-nodes/runnable.tpp>
#include <impl/ast-nodes/pragma.tpp>
#include <impl/ast-nodes/dump.tpp>
#include <impl/ast-nodes/embed.tpp>
#include <impl/ast-nodes/set.tpp>
#include <impl/ast-nodes/unset.tpp>

#include <impl/ast-nodes/for.tpp>
#include <impl/ast-nodes/foreach.tpp>
#include <impl/ast-nodes/foreachi.tpp>
#include <impl/ast-nodes/conditional.tpp>

#include <impl/ast-nodes/builtin.tpp>
#include <impl/ast-nodes/varsubst.tpp>
#include <impl/ast-nodes/macrocall.tpp>
#include <impl/ast-nodes/unaryop.tpp>
#include <impl/ast-nodes/binaryop.tpp>
#include <impl/ast-nodes/string.tpp>
#include <impl/ast-nodes/integer.tpp>
#include <impl/ast-nodes/floating.tpp>
#include <impl/ast-nodes/boolean.tpp>
#include <impl/ast-nodes/list.tpp>

#include <impl/ast-nodes/text.tpp>

namespace supdef::ast
{
    class parse_error
    {
    public:
        parse_error(const ::supdef::token_loc& loc, const std::string& msg)
            : m_loc(loc)
            , m_msg(msg)
        {
        }
        parse_error(const ::supdef::token_loc& loc, std::string&& msg)
            : m_loc(loc)
            , m_msg(std::move(msg))
        {
        }
        parse_error(::supdef::token_loc&& loc, const std::string& msg)
            : m_loc(std::move(loc))
            , m_msg(msg)
        {
        }
        parse_error(::supdef::token_loc&& loc, std::string&& msg)
            : m_loc(std::move(loc))
            , m_msg(std::move(msg))
        {
        }

        const ::supdef::token_loc& location() const
        {
            return m_loc;
        }

        const std::string& message() const
        {
            return m_msg;
        }

    private:
        ::supdef::token_loc m_loc;
        std::string m_msg;
    };

    class builder
    {
    public:
        builder(const std::list<token>& tokens);
        builder(const std::vector<token>& tokens);
        ~builder() = default;

        std::generator<std::expected<shared_node, parse_error>> build();

    private:
        std::variant<const std::list<token>*, const std::vector<token>*> m_tokens;
    };
}

#endif

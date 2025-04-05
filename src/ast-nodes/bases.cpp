#include <ast.hpp>
#include <impl/parsing-utils.tpp>

namespace supdef::ast
{
    /*
     * supdef::ast::node::kind
     */
    node::kind operator~(node::kind k) noexcept
    {
        return static_cast<node::kind>(~std::to_underlying(k));
    }
        
    node::kind operator<<(node::kind k, std::make_unsigned_t<std::underlying_type_t<node::kind>> n) noexcept
    {
        return static_cast<node::kind>(std::to_underlying(k) << n);
    }
        
    node::kind operator>>(node::kind k, std::make_unsigned_t<std::underlying_type_t<node::kind>> n) noexcept
    {
        return static_cast<node::kind>(std::to_underlying(k) >> n);
    }

    std::ostream& operator<<(std::ostream& os, const node::kind& k)
    {
        using namespace std::literals::string_view_literals;
        return os << strip(magic_enum::enum_name(k), " \t_") << " node"sv;
    }

    /*
     * supdef::ast::node
     */
    node::node(::supdef::token_loc&& loc) noexcept
        : m_loc(std::move(loc))
    {
    }

    node::node(const ::supdef::token_loc& loc) noexcept
        : m_loc(loc)
    {
    }

    node::kind node::node_kind() const noexcept
    {
        return kind::unknown;
    }

    bool node::operator==(kind k) const noexcept
    {
        return this->is(k);
    }

    node::kind node::operator&(node::kind k) const noexcept
    {
        return this->node_kind() & k;
    }

    node::kind node::operator|(node::kind k) const noexcept
    {
        return this->node_kind() | k;
    }

    node::kind node::operator^(node::kind k) const noexcept
    {
        return this->node_kind() ^ k;
    }

    node::kind node::operator~() const noexcept
    {
        return ~this->node_kind();
    }

    std::ostream& node::output_to(std::ostream& os) const
    {
        return this->do_output_to(os);
    }

    const ::supdef::token_loc& node::location() const & noexcept
    {
        return m_loc;
    }

    ::supdef::token_loc&& node::location() && noexcept
    {
        return std::move(m_loc);
    }

    std::ostream& operator<<(std::ostream& os, const node& n)
    {
        return n.output_to(os);
    }

    /*
     * supdef::ast::directive_node
     */

    directive_node::directive_node(bool outputs) noexcept
        : m_outputs(outputs)
    {
    }

    directive_node::directive_node(::supdef::token_loc&& loc, bool outputs) noexcept
        : node(std::move(loc))
        , m_outputs(outputs)
    {
    }

    bool directive_node::outputs() const noexcept
    {
        return m_outputs;
    }

    node::kind directive_node::node_kind() const noexcept
    {
        return kind::directive;
    }

    /*
     * supdef::ast::expression_node
     */
    void expression_node::requires_constant() const
    {
        if (!this->is_constant())
            throw std::runtime_error("non-constant coercion needs interpreter to first evaluate expression");
    }

    expression_node::expression_node(::supdef::token_loc&& loc) noexcept
        : node(std::move(loc))
    {
    }

    node::kind expression_node::node_kind() const noexcept
    {
        return kind::expression;
    }

    /*
     * supdef::ast::block_node
     */
    block_node::block_node(std::vector<std::vector<shared_node>>&& repl) noexcept
        : m_replacements(std::move(repl))
    {
    }

    block_node::block_node(::supdef::token_loc&& loc, std::vector<std::vector<shared_node>>&& repl) noexcept
        : node(std::move(loc))
        , m_replacements(std::move(repl))
    {
    }

    const std::vector<std::vector<shared_node>>& block_node::replacements() const noexcept
    {
        return m_replacements;
    }

    const std::vector<shared_node>& block_node::replacement_for(size_t index) const
    {
        return m_replacements.at(index);
    }

    const std::vector<shared_node>& block_node::replacement() const
    {
        return m_replacements.at(0);
    }

    node::kind block_node::node_kind() const noexcept
    {
        return kind::block;
    }

    // output functions
    std::ostream& directive_node::do_output_to(std::ostream& os) const
    {
        os << "directive node: " << magic_enum::enum_name(this->node_kind()).data() << '\n';
        return os;
    }

    std::ostream& expression_node::do_output_to(std::ostream& os) const
    {
        os << "expression node: " << magic_enum::enum_name(this->node_kind()).data() << '\n';
        return os;
    }

    std::ostream& block_node::do_output_to(std::ostream& os) const
    {
        os << "block node: " << magic_enum::enum_name(this->node_kind()).data() << '\n';
        return os;
    }
}
namespace supdef::ast
{
    class directive_node
        : public virtual node
    {
    protected:
        directive_node(bool outputs)
            : m_outputs(outputs)
        {
        }

    public:
        directive_node(::supdef::token_loc&& loc, bool outputs)
            : node(std::move(loc))
            , m_outputs(outputs)
        {
        }

        bool outputs() const
        {
            return m_outputs;
        }

        virtual kind node_kind() const override
        {
            return kind::directive;
        }

    private:
        bool m_outputs;
    };

    class expression_node
        : public virtual node
    {
    protected:
        expression_node() = default;

    public:
        expression_node(::supdef::token_loc&& loc)
            : node(std::move(loc))
        {
        }

        virtual kind node_kind() const override
        {
            return kind::expression;
        }
    };

    class block_node
        : public virtual node
    {
    protected:
        block_node(std::vector<std::vector<shared_node>>&& repl)
            : m_replacements(std::move(repl))
        {
        }

    public:
        block_node(::supdef::token_loc&& loc, std::vector<std::vector<shared_node>>&& repl)
            : node(std::move(loc))
            , m_replacements(std::move(repl))
        {
        }

        const std::vector<std::vector<shared_node>>& replacements() const
        {
            return m_replacements;
        }
        const std::vector<shared_node>& replacement_for(size_t index) const
        {
            return m_replacements.at(index);
        }
        const std::vector<shared_node>& replacement() const
        {
            return m_replacements.front();
        }

        virtual kind node_kind() const override
        {
            return kind::block;
        }

    private:
        std::vector<std::vector<shared_node>> m_replacements;
    };
}
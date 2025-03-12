#ifndef PARSER_HPP
#define PARSER_HPP

#include <types.hpp>
#include <printer.hpp>
#include <stack.hpp>
#include <interpreter.hpp>
#include <file.hpp>
#include <unicode.hpp>
#include <tokenizer.hpp>
#include <ast-fwd.hpp>
#include <detail/xxhash.hpp>

#include <bits/stdc++.h>

#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/test/debug.hpp>

#include <magic_enum.hpp>

#include <simdutf.h>

#include <unicode/ustream.h>
#include <unicode/unistr.h>

#include <boost/logic/tribool.hpp>

namespace supdef
{
    class parser;

    struct parser_compare
    {
        using ret_type = decltype(std::declval<stdfs::path>() <=> std::declval<stdfs::path>());

        enum class op
        {
            less,
            greater,
            equal
        };
    protected:
        static bool do_less(const stdfs::path& lhs, const stdfs::path& rhs);

    public:
        static bool operator()(const parser& lhs, const parser& rhs, op op = op::less);
    };

    class parser
    {
        friend struct parser_compare;

    public:
        enum output_kind : uint_fast8_t
        {
            text      = 1U << 0,
            tokens    = 1U << 1,
            ast       = 1U << 2,
            imports   = 1U << 3,
            supdefs   = 1U << 4,
            recursive = 1U << 6,
            original  = 1U << 7,

            recursive_supdefs = supdefs
                              | recursive,
            all               = text
                              | tokens
                              | ast
                              | imports
                              | supdefs
                              | recursive
                              | original
        };
    
    protected:
        [[__nodiscard__]]
        bool add_child_parser(const stdfs::path& filename, token_kind pathtype) noexcept;

#if 0
        static std::optional<std::pair<registered_supdef::options, std::u32string>>
        parse_supdef_start(
            std::list<token>::const_iterator line_start,
            std::list<token>::const_iterator line_end,
            const std::u32string& origdata
        );

        static std::optional<
            std::tuple<registered_runnable::lang, registered_runnable::options, std::u32string>
        >
        parse_runnable_start(
            std::list<token>::const_iterator line_start,
            std::list<token>::const_iterator line_end,
            const std::u32string &origdata
        );
#endif

        static bool
        parse_supdef_runnable_end(
            std::list<token>::const_iterator line_start,
            std::list<token>::const_iterator line_end,
            const std::u32string& origdata
        );

#if 0
        std::optional<registered_supdef> get_supdef(const std::u32string& name, bool recurse = true) const noexcept;
        
        void execute_toplevel();
        
        std::list<token>::iterator
        execute_variable_substitution(std::list<token>::iterator tok, const std::list<token>::iterator tokcpy);

        std::list<token>::iterator
        execute_directive(std::list<token>::iterator tok, const std::list<token>::iterator tokcpy);

        /* std::list<token>::iterator */
        void
        execute_pragma(const std::list<token>::iterator start, const std::list<token>::iterator end);
#endif

    public:
        parser(const stdfs::path& filename);
        parser(stdfs::path&& filename);
        ~parser();

        // tokenize the file
        void do_stage1();
        // splice lines
        void do_stage2();
        // remove comments
        void do_stage3();
        // process imports
        void do_stage4();
        // retrieve supdefs and runnables
        void do_stage5();
        // substitute supdefs, runnables or builtin functions (e.g. @math, @len, ...) calls
        // substitute @set variables
        // parse @pragma's
        void do_stage6();
        // process @embed and @dump calls
        void do_stage7();

        void output_to(std::ostream& os, output_kind kind = text);
        void output_to(const std::filesystem::path& filename, output_kind kind = text);

        bool operator==(const parser& rhs) const
        {
            auto thispath = *m_file.filename();
            auto rhspath  = *rhs.m_file.filename();
            assert(thispath.is_absolute());
            assert(rhspath.is_absolute());
            return thispath == rhspath;
        }
        parser_compare::ret_type operator<=>(const parser& rhs) const
        {
            auto thispath = *m_file.filename();
            auto rhspath  = *rhs.m_file.filename();
            assert(thispath.is_absolute());
            assert(rhspath.is_absolute());
            return thispath <=> rhspath;
        }
    
    private:
        using supdef_map_type   = umap<std::u32string, shared_ptr<supdef::ast::supdef_node>>;
        using runnable_map_type = umap<std::u32string, shared_ptr<supdef::ast::runnable_node>>;
        /* stack<substitution_context> m_ctx; */
        source_file m_file;
        std::list<token> m_tokens;
        std::set<parser> m_imported_parsers;
        supdef_map_type m_supdefs;
        runnable_map_type m_runnables;
    };

    
}
bool supdef::parser_compare::do_less(const stdfs::path& lhs, const stdfs::path& rhs)
{
    return lhs < rhs;
}

bool supdef::parser_compare::operator()(
    const supdef::parser& lhs, const supdef::parser& rhs,
    supdef::parser_compare::op op
) {
    if (op == supdef::parser_compare::op::less)
        return supdef::parser_compare::do_less(*lhs.m_file.filename(), *rhs.m_file.filename());
    std::unreachable(); // not implemented
}

#endif
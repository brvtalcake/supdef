#ifndef DIRECTIVES_HPP
#define DIRECTIVES_HPP

#include <types.hpp>
#include <unicode.hpp>
#include <file.hpp>

#include <variant>
#include <string>
#include <optional>

#if __cpp_explicit_this_parameter < 202110L
    #error "explicit this parameter is required but not available"
#endif

namespace supdef
{
    class directive
    {
    public:
        enum class type
        {
            /* top-level */
            import,   // @import
            define,   // @supdef / @end
            runnable, // @runnable / @end
            embed,    // @embed
            dump,     // @dump

            /* in supdef's */
            let,     // @let
            func,    // @<func-name>(<args>)
            cond,    // @if / @elseif / @else / @endif
            forloop  // @for
        };

        directive(type t, const std::u32string& d) noexcept
            : m_data(d), m_type(t)
        {
        }
        directive(type t, std::u32string&& d) noexcept
            : m_data(std::move(d)), m_type(t)
        {
        }

        const std::u32string& get_data() const noexcept
        {
            return m_data;
        }

        type get_type() const noexcept
        {
            return m_type;
        }

    private:
        std::u32string m_data;
        type m_type;
    };

    using source_unit = std::variant<directive, std::u32string>;

    class function_directive : public directive
    {
    public:
        enum class func
        {
            join,  // @join(<tok>, <tok>, ...)
            split, // @split(<tok>, <index>)
            str,   // @str(<tok>)
            unstr, // @unstr(<str>)
            len,   // @len(<tok> | <str>)
            math   // @math(<expr>)
        };

        function_directive(
            std::convertible_to<const std::u32string> auto&& content,
            func fn,
            std::convertible_to<const std::u32string> auto&& params
        ) noexcept(
            std::is_nothrow_convertible_v<decltype(content), const std::u32string> &&
            std::is_nothrow_convertible_v<decltype(params),  const std::u32string>
        ) : directive(directive::type::func, std::forward<decltype(content)>(content))
          , m_params(std::forward<decltype(params)>(params))
          , m_func(fn)
        {
        }
        
        const std::u32string& get_params() const noexcept
        {
            return m_params;
        }

        func get_func() const noexcept
        {
            return m_func;
        }

    private:
        std::u32string m_params;
        func m_func;
    };

    class import_directive : public directive
    {
    public:
        import_directive(
            std::convertible_to<const std::u32string> auto&& content,
            std::convertible_to<const stdfs::path> auto&& path
        ) noexcept(
            std::is_nothrow_convertible_v<decltype(content), std::u32string> &&
            std::is_nothrow_convertible_v<decltype(path), stdfs::path>
        ) : directive(directive::type::import, std::forward<decltype(content)>(content))
          , m_path(std::forward<decltype(path)>(path))
        {
        }

        const stdfs::path& get_path() const noexcept
        {
            return m_path;
        }
        source_file get_file() const
        {
            return source_file(m_path);
        }

    private:
        stdfs::path m_path;
    };

    class define_directive : public directive
    {
    public:
        enum options
        {
            none = 0
        };
        define_directive(
            std::convertible_to<const std::u32string> auto&& content,
            options opts,
            std::convertible_to<const std::u32string> auto&& name
        ) noexcept(
            std::is_nothrow_convertible_v<decltype(content), std::u32string> &&
            std::is_nothrow_convertible_v<decltype(name   ), std::u32string>
        ) : directive(directive::type::define, std::forward<decltype(content)>(content))
          , m_name(std::forward<decltype(name)>(name))
          , m_opts(opts)
        {
        }

        const std::u32string& get_name() const noexcept
        {
            return m_name;
        }

        options get_options() const noexcept
        {
            return m_opts;
        }

    private:
        std::u32string m_name;
        options m_opts;
    };
}

#endif

#ifndef EXPANDABLESV_HPP
#define EXPANDABLESV_HPP

#include <types.hpp>

#include <memory>
#include <string_view>
#include <string>
#include <ranges>
#include <concepts>

namespace supdef
{
    // TODO: implement this
#if 0
    template <typename... StringViewArgsT>
    class expandable_basic_string_view
        : public stdranges::view_interface<expandable_basic_string_view<StringViewArgsT...>>
    {
    protected:
        using sv_type = std::basic_string_view<StringViewArgsT...>;

    public:
        constexpr expandable_basic_string_view() noexcept = default;

        constexpr expandable_basic_string_view(const expandable_basic_string_view&) noexcept = delete;
        constexpr expandable_basic_string_view(expandable_basic_string_view&&) noexcept = default;

        constexpr expandable_basic_string_view& operator=(const expandable_basic_string_view&) noexcept = delete;
        constexpr expandable_basic_string_view& operator=(expandable_basic_string_view&&) noexcept = default;

        constexpr explicit expandable_basic_string_view(sv_type sv) noexcept
            : m_sv{ sv }
            , m_start{ &m_sv }
            , m_prev{ nullptr }
            , m_next{ nullptr }
        {
        }

        constexpr expandable_basic_string_view& operator=(sv_type sv) noexcept
        {
            *this = expandable_basic_string_view{ sv };
        }

    private:
        sv_type  m_sv;
        sv_type* m_start;
        expandable_basic_string_view* m_prev;
        expandable_basic_string_view* m_next;
    };
#endif
}

#endif

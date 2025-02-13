#ifdef __INTELLISENSE__
# include <parser.hpp>
#endif

namespace
{
    struct ascii_to_char32_view
        : public stdranges::view_interface<ascii_to_char32_view>
    {
        struct iterator
        {
            using iterator_concept = std::random_access_iterator_tag;
            using iterator_category = std::random_access_iterator_tag;
            using value_type = std::iterator_traits<std::u32string_view::iterator>::value_type;
            using difference_type = std::iterator_traits<std::u32string_view::iterator>::difference_type;
            using pointer = std::iterator_traits<std::u32string_view::iterator>::pointer;
            using reference = std::iterator_traits<std::u32string_view::iterator>::reference;

            constexpr iterator() noexcept = default;
            constexpr iterator(const iterator&) noexcept = default;
            constexpr iterator(iterator&&) noexcept = default;
            constexpr iterator& operator=(const iterator&) noexcept = default;
            constexpr iterator& operator=(iterator&&) noexcept = default;

            constexpr iterator(std::string::iterator iter) noexcept
                : m_iter{ std::move(iter) }
            {
            }

            constexpr iterator& operator++() noexcept
            {
                stdranges::advance(m_iter, 1);
                return *this;
            }

            constexpr iterator operator++(int) noexcept
            {
                auto cpy = *this;
                stdranges::advance(m_iter, 1);
                return cpy;
            }

            constexpr iterator& operator+=(difference_type n) noexcept
            {
                stdranges::advance(m_iter, n);
                return *this;
            }

            constexpr iterator operator+(difference_type n) noexcept
            {
                auto cpy = *this;
                stdranges::advance(m_iter, n);
                return cpy;
            }

            constexpr iterator& operator--() noexcept
            {
                stdranges::advance(m_iter, -1);
                return *this;
            }

            constexpr iterator operator--(int) noexcept
            {
                auto cpy = *this;
                stdranges::advance(m_iter, -1);
                return cpy;
            }

            constexpr iterator& operator-=(difference_type n) noexcept
            {
                stdranges::advance(m_iter, -n);
                return *this;
            }

            constexpr iterator operator-(difference_type n) noexcept
            {
                auto cpy = *this;
                stdranges::advance(m_iter, -n);
                return cpy;
            }

            constexpr reference operator*() const noexcept
            {
                m_buf = static_cast<char32_t>(*m_iter);
                return m_buf;
            }

            constexpr pointer operator->() const noexcept
            {
                return &m_buf;
            }

            constexpr auto operator<=>(const iterator& rhs) const noexcept
            {
                return m_iter <=> rhs.m_iter;
            }
        private:
            std::string_view::iterator m_iter;
            char32_t m_buf;
        };
        
        constexpr ascii_to_char32_view(std::string& str) noexcept
            : m_str{ str }
        {
        }
        constexpr ascii_to_char32_view(std::string_view sv) noexcept
            : m_str{ sv }
        {
        }

        constexpr ascii_to_char32_view(const ascii_to_char32_view&) noexcept = default;
        constexpr ascii_to_char32_view(ascii_to_char32_view&&) noexcept = default;

        constexpr ascii_to_char32_view& operator=(const ascii_to_char32_view&) noexcept = default;
        constexpr ascii_to_char32_view& operator=(ascii_to_char32_view&&) noexcept = default;

        constexpr iterator begin() const noexcept
        {
            return iterator{ m_str.begin() };
        }

        constexpr iterator end() const noexcept
        {
            return iterator{ m_str.end() };
        }

    private:
        std::string_view m_str;
    };

    static bool strmatch(
        const std::u32string_view str, const std::u32string_view pattern,
        bool case_sensitive = true
    )
    {
        if (str.size() != pattern.size())
            return false;
        if (case_sensitive)
            return str == pattern;
        icu::UnicodeString ustr = icu::UnicodeString::fromUTF32(
            reinterpret_cast<const UChar32*>(str.data()),
            str.size()
        );
        icu::UnicodeString upattern = icu::UnicodeString::fromUTF32(
            reinterpret_cast<const UChar32*>(pattern.data()),
            pattern.size()
        );
        return ustr.caseCompare(upattern, U_FOLD_CASE_DEFAULT) == 0;
    }

    static bool strmatch(
        const std::u32string_view str, const ascii_to_char32_view pattern,
        bool case_sensitive = true
    )
    {
        if (str.size() != stdranges::size(pattern))
            return false;
        std::u32string patternstr = std::u32string( stdranges::begin(pattern), stdranges::end(pattern) );
        if (case_sensitive)
            return str == std::u32string_view{ patternstr };
        icu::UnicodeString ustr = icu::UnicodeString::fromUTF32(
            reinterpret_cast<const UChar32*>(str.data()),
            str.size()
        );
        icu::UnicodeString upattern = icu::UnicodeString::fromUTF32(
            reinterpret_cast<const UChar32*>(patternstr.data()),
            patternstr.size()
        );
        return ustr.caseCompare(upattern, U_FOLD_CASE_DEFAULT) == 0;
    }

    template <size_t N>
    struct bitset_inserter
    {
        using iterator_category = std::output_iterator_tag;
        using iterator_concept = std::output_iterator_tag;
        using value_type = void;
        using difference_type = ptrdiff_t;
        using pointer = void;
        using reference = std::bitset<N>::reference;

        constexpr bitset_inserter(std::bitset<N>& bs) noexcept
            : m_bs{ std::addressof(bs) }
            , m_idx{ 0 }
        {
        }

        constexpr bitset_inserter(const bitset_inserter&) noexcept = default;
        constexpr bitset_inserter(bitset_inserter&&) noexcept = default;

        constexpr bitset_inserter& operator=(const bitset_inserter&) noexcept = default;
        constexpr bitset_inserter& operator=(bitset_inserter&&) noexcept = default;

#if 0
        constexpr bitset_inserter& operator*() noexcept
        {
            return *this;
        }

        constexpr bitset_inserter& operator=(bool value) noexcept
        {
            (*m_bs)[m_idx] = value;
            return *this;
        }
#else
        constexpr reference operator*() noexcept
        {
            reference ref = (*m_bs)[m_idx];
            return ref;
        }
#endif

        constexpr bitset_inserter& operator++() noexcept
        {
            m_idx++;
            return *this;
        }

        constexpr bitset_inserter operator++(int) noexcept
        {
            auto cpy = *this;
            m_idx++;
            return cpy;
        }

    private:
        std::bitset<N>* m_bs;
        size_t m_idx;
    };
}

constexpr supdef::parser::registered_supdef::options
supdef::parser::registered_supdef::parse_options(const std::u32string& str)
{
    using namespace std::string_view_literals;
    using opts_type = ::supdef::parser::registered_supdef::options;

    opts_type opts;
    std::u32string_view substr{str};

    while (!substr.empty())
    {
        auto pos = substr.find(U',');
        if (pos == std::u32string_view::npos)
            pos = substr.size();

        auto token = substr.substr(0, pos);

        auto NLstate = parse_bool_opt(token, U"NL"sv);
        auto nlstate = parse_bool_opt(token, U"nl"sv);
        if (NLstate == false || nlstate == false)
            opts.eat_newlines = false;
        else if (NLstate == true || nlstate == true)
            opts.eat_newlines = true;

        substr.remove_prefix(pos + 1);
    }

    return static_cast<opts_type>(opts);
}

constexpr boost::logic::tribool supdef::registered_base::parse_bool_val(
    std::u32string_view sv
)
{
    using namespace std::string_view_literals;

    if (
        (sv.size() == 1 && ::supdef::unicat::is_digit(sv.front()) && ::supdef::unicode::numeric_value(sv.front()) == 0) ||
        strmatch(sv, U"false"sv, false) || strmatch(sv, U"no"sv, false) || strmatch(sv, U"off"sv, false)
    )
        return false;
    if (
        (sv.size() == 1 && ::supdef::unicat::is_digit(sv.front()) && ::supdef::unicode::numeric_value(sv.front()) != 0) ||
        strmatch(sv, U"true"sv, false) || strmatch(sv, U"yes"sv, false) || strmatch(sv, U"on"sv, false)
    )
        return true;
    return boost::logic::indeterminate;
}

constexpr boost::logic::tribool supdef::registered_base::parse_bool_opt(
    std::u32string_view sv, std::u32string_view opt)
{
    using namespace std::string_view_literals;

    if (!sv.starts_with(opt))
        return boost::logic::indeterminate;
    sv.remove_prefix(opt.size());
    if (sv.empty())
        return true;

    if (sv.front() != U'=')
        return boost::logic::indeterminate;
    sv.remove_prefix(1);

    if (sv.empty())
        return boost::logic::indeterminate;

    return parse_bool_val(sv);
}

constexpr std::optional<supdef::parser::registered_runnable::lang>
supdef::parser::registered_runnable::is_lang_identifier(std::u32string_view sv)
{
    constexpr std::array langs = magic_enum::enum_values<::supdef::parser::registered_runnable::lang::identifier>();
    static_assert(langs.size() == ::supdef::parser::registered_runnable::lang::_langidentcount);
    std::bitset<langs.size()> matches;
    stdranges::transform(
        langs, bitset_inserter{matches},
        // op
        [&sv](const ascii_to_char32_view& langident) {
            return strmatch(sv, langident, false);
        },
        // projection of range `langs`
        [](const std::string_view& original_langident) {
            return ascii_to_char32_view{ original_langident };
        }
    );
}
#ifdef __INTELLISENSE__
# include <parser.hpp>
#endif

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/transform_view.hpp>

namespace
{
    template <typename CharT>
    static constexpr inline std::basic_string<CharT> strip(
        const std::basic_string<CharT>& str,
        const std::basic_string<CharT>& chars
    ) {
        auto start = str.find_first_not_of(chars);
        if (start == std::basic_string<CharT>::npos)
            return std::basic_string<CharT>();
        auto end = str.find_last_not_of(chars);
        if (end == std::basic_string<CharT>::npos)
            return std::basic_string<CharT>();
        return str.substr(start, end - start + 1);
    }

    template <typename CharT>
    static constexpr inline std::basic_string<CharT> translate(
        const std::basic_string<CharT>& str,
        const std::basic_string<CharT>& from_chars,
        const std::basic_string<CharT>& to_chars
    ) {
        std::basic_string<CharT> result;
        result.reserve(str.size());
        for (auto c : str)
        {
            auto pos = from_chars.find(c);
            if (pos != std::basic_string<CharT>::npos)
                result.push_back(to_chars[pos]);
            else
                result.push_back(c);
        }
        return result;
    }

    std::list<::supdef::token> isolate_line(
        std::list<::supdef::token>& tokens,
        std::list<::supdef::token>::iterator& pos
    ) noexcept
    {
        std::list<::supdef::token> line;
        auto line_start = reverse_find_if(
            pos, tokens.begin(),
            [](const ::supdef::token& tok) {
                return tok.kind == ::supdef::token_kind::newline;
            }
        );
        auto line_end = stdranges::find_if(
            pos, tokens.end(),
            [](const ::supdef::token& tok) {
                return tok.kind == ::supdef::token_kind::newline;
            }
        );
        line.splice(line.begin(), tokens, line_start, line_end);
        pos = line_end;
        return line;
    }

    template <typename T, size_t N>
    constexpr std::array<T, N> mk_array(const T(&arr)[N]) noexcept
    {
        return std::to_array(arr);
    }

    template <typename T, typename... Args, size_t N = sizeof...(Args)>
    constexpr std::array<T, N> mk_array(Args&&... args) noexcept
    {
        T arr[N] = { std::forward<Args>(args)... };
        return std::to_array(arr);
    }

    template <typename T, typename... Args, size_t... Is, size_t N = sizeof...(Args)>
    constexpr std::array<T, N> mk_array(const std::tuple<Args...>& tup, std::index_sequence<Is...>) noexcept
    {
        return std::array<T, N>{ std::get<Is>(tup)... };
    }

    template <typename T, typename... Args, size_t... Is, size_t N = sizeof...(Args)>
    constexpr std::array<T, N> mk_array(std::tuple<Args...>&& tup, std::index_sequence<Is...>) noexcept
    {
        return std::array<T, N>{ std::move(std::get<Is>(tup))... };
    }

    template <typename T, typename... Args, size_t N = sizeof...(Args)>
    constexpr std::array<T, N> mk_array(const std::tuple<Args...>& tup) noexcept
    {
        return mk_array<T>(tup, std::index_sequence_for<Args...>{});
    }

    template <typename T, typename... Args, size_t N = sizeof...(Args)>
    constexpr std::array<T, N> mk_array(std::tuple<Args...>&& tup) noexcept
    {
        return mk_array<T>(std::move(tup), std::index_sequence_for<Args...>{});
    }

    template <typename... Types>
    struct typelist
    {
        using types = std::tuple<Types...>;
    };

    template <typename T, size_t N>
    static constexpr auto as_tuple(const std::array<T, N>& arr) noexcept
    {
        return std::apply([](auto&&... args) {
            return std::make_tuple(std::forward<decltype(args)>(args)...);
        }, arr);
    }

    template <typename T, size_t N>
    static constexpr auto as_tuple(std::array<T, N>&& arr) noexcept
    {
        return std::apply([](auto&&... args) {
            return std::make_tuple(std::forward<decltype(args)>(args)...);
        }, std::move(arr));
    }

    template <typename... T1Types, typename... T2Types, size_t... Is>
    static constexpr auto zip_impl(
        const std::tuple<T1Types...>& t1, const std::tuple<T2Types...>& t2,
        std::index_sequence<Is...>
    ) -> std::tuple<std::tuple<T1Types, T2Types>...>
    {
        return std::make_tuple( std::make_tuple( std::get<Is>(t1), std::get<Is>(t2) )... );
    }

    template <typename... T1Types, typename... T2Types>
        requires (sizeof...(T1Types) == sizeof...(T2Types))
    static constexpr auto zip(
        const std::tuple<T1Types...>& t1, const std::tuple<T2Types...>& t2
    ) -> std::tuple<std::tuple<T1Types, T2Types>...>
    {
        return zip_impl(t1, t2, std::index_sequence_for<T1Types...>{});
    }

    template <typename... T1Types, typename... T2Types, typename... OtherTypes>
        requires (sizeof...(T1Types) == sizeof...(T2Types))
    static constexpr auto zip(
        const std::tuple<T1Types...>& t1, const std::tuple<T2Types...>& t2,
        OtherTypes&&... others
    ) -> decltype(zip(zip(t1, t2), std::forward<OtherTypes>(others)...))
    {
        return zip(zip(t1, t2), std::forward<OtherTypes>(others)...);
    }
        
    static constexpr auto find_closing(
        std::input_iterator auto start, std::input_iterator auto end,
        std::invocable<std::iter_value_t<decltype(start)>> auto is_open,
        std::invocable<std::iter_value_t<decltype(start)>> auto is_close
    ) requires std::same_as<std::invoke_result_t<decltype(is_open),  std::iter_value_t<decltype(start)>>, bool> &&
               std::same_as<std::invoke_result_t<decltype(is_close), std::iter_value_t<decltype(start)>>, bool> &&
               std::same_as<decltype(start), decltype(end)>
    {
        long long depth = 1;
        while (start != end)
        {
            if (is_open(*start))
                depth++;
            else if (is_close(*start))
                depth--;
            if (depth == 0)
                return start;
            stdranges::advance(start, 1);
        }
        return end;
    }

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

            constexpr iterator(std::string_view::iterator iter) noexcept
                : m_iter( iter )
                , m_buf( 0 )
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

            friend constexpr iterator operator+(const iterator& it, difference_type n) noexcept
            {
                return iterator{ it.m_iter + n };
            }
            friend constexpr iterator operator+(difference_type n, const iterator& it) noexcept
            {
                return iterator{ it.m_iter + n };
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

            friend constexpr iterator operator-(const iterator& it, difference_type n) noexcept
            {
                return iterator{ it.m_iter - n };
            }

            constexpr difference_type operator-(const iterator& other) const noexcept
            {
                return m_iter - other.m_iter;
            }

            constexpr reference operator*() const noexcept
            {
                m_buf = static_cast<char32_t>(*m_iter);
                return m_buf;
            }

            constexpr pointer operator->() const noexcept
            {
                m_buf = static_cast<char32_t>(*m_iter);
                return &m_buf;
            }

            constexpr reference operator[](difference_type n) const noexcept
            {
                m_buf = *(*this + n);
                return m_buf;
            }

            friend constexpr std::strong_ordering operator<=>(const iterator& lhs, const iterator& rhs) noexcept
            {
                return lhs.m_iter <=> rhs.m_iter;
            }

            friend constexpr bool operator==(const iterator& lhs, const iterator& rhs) noexcept
            {
                return lhs.m_iter == rhs.m_iter;
            }
        private:
            std::string_view::iterator m_iter;
            mutable char32_t m_buf;
        };
        
        constexpr ascii_to_char32_view(std::string& str) noexcept
            : m_sv{ str }
        {
        }
        constexpr ascii_to_char32_view(std::string_view sv) noexcept
            : m_sv{ sv }
        {
        }

        constexpr ascii_to_char32_view(const ascii_to_char32_view&) noexcept = default;
        constexpr ascii_to_char32_view(ascii_to_char32_view&&) noexcept = default;

        constexpr ascii_to_char32_view& operator=(const ascii_to_char32_view&) noexcept = default;
        constexpr ascii_to_char32_view& operator=(ascii_to_char32_view&&) noexcept = default;

        constexpr iterator begin() const noexcept
        {
            return iterator{ m_sv.begin() };
        }

        constexpr iterator end() const noexcept
        {
            return iterator{ m_sv.end() };
        }

    private:
        std::string_view m_sv;
    };

    static_assert(std::three_way_comparable<ascii_to_char32_view::iterator>);
    static_assert(stdranges::forward_range<ascii_to_char32_view>);
    static_assert(stdranges::random_access_range<ascii_to_char32_view>);
    static_assert(std::sized_sentinel_for<ascii_to_char32_view::iterator, ascii_to_char32_view::iterator>);

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
        
        class reference
        {
            friend struct bitset_inserter;

        public:
            constexpr reference() noexcept = default;

            constexpr reference& operator=(bool value) noexcept
            {
                (*m_bs_ref)[*m_idx_ref] = value;
                return *this;
            }

            constexpr const reference& operator=(bool value) const noexcept
            {
                (*m_bs_ref)[*m_idx_ref] = value;
                return *this;
            }

            constexpr operator bool() const noexcept
            {
                return (*m_bs_ref)[*m_idx_ref];
            }

        private:

            reference(std::bitset<N>* bs, size_t* idx) noexcept
                : m_bs_ref{ bs }
                , m_idx_ref{ idx }
            {
            }

            std::bitset<N>* m_bs_ref;
            size_t* m_idx_ref;
        };

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
            reference ref{ m_bs, &m_idx };
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

    template <typename IterT>
    concept points_to_token = std::same_as<typename std::iterator_traits<IterT>::value_type, ::supdef::token>;

    template <typename IterT>
    concept points_to_token_and_bidir = points_to_token<IterT> && std::bidirectional_iterator<IterT>;

    template <typename IterT>
    concept points_to_token_and_fwd = points_to_token<IterT> && std::forward_iterator<IterT>;

    template <typename IterT>
    concept points_to_token_and_input = points_to_token<IterT> && std::input_iterator<IterT>;

    template <typename FnT, typename IterT>
    concept predicate = std::predicate<FnT, ::supdef::token> ||
                        std::predicate<FnT, IterT>;
    
    template <typename FnT, typename IterT>
    concept invokeable = std::invocable<FnT, ::supdef::token> ||
                         std::invocable<FnT, IterT>;

    static bool at_start_of_line(const points_to_token_and_bidir auto iter, const points_to_token_and_bidir auto begin)
    {
        auto cpy = iter;
        if (cpy == begin)
            return true;
        stdranges::advance(cpy, -1);
        while (cpy != begin && cpy->kind != ::supdef::token_kind::newline)
        {
            if (cpy->kind != ::supdef::token_kind::horizontal_whitespace)
                return false;
            stdranges::advance(cpy, -1);
        }
        return true;
    }

    static auto reverse_find(
        const std::bidirectional_iterator auto begin, const std::bidirectional_iterator auto end,
        auto&& value
    )
    {
        auto iter = end;
        while (iter != begin)
        {
            stdranges::advance(iter, -1);
            if (*iter == value)
                return iter;
        }
        return iter;
    }

    static auto reverse_find_if(
        const std::bidirectional_iterator auto begin, const std::bidirectional_iterator auto end,
        auto&& pred
    )
    {
        auto iter = end;
        while (iter != begin)
        {
            stdranges::advance(iter, -1);
            if (pred(*iter))
                return iter;
        }
        return iter;
    }

    static auto replace_from_to(
        auto& destcont, std::input_iterator auto destfirst, std::input_iterator auto destlast,
        std::input_iterator auto srcfirst, std::input_iterator auto srclast
    )
    {
        destcont.erase(destfirst, destlast);
        return stdranges::copy(srcfirst, srclast, std::inserter(destcont, destlast));
    }

    static auto replace_from_to(
        auto& destcont, std::input_iterator auto destfirst, std::input_iterator auto destlast,
        stdranges::input_range auto&& srcrange
    )
    {
        destcont.erase(destfirst, destlast);
        return stdranges::copy(
            std::forward<std::remove_reference_t<decltype(srcrange)>>(srcrange),
            std::inserter(destcont, destlast)
        );
    }

    static auto replace_from_to(
        auto& destcont, std::input_iterator auto destfirst, std::input_iterator auto destlast,
        auto&& srcelem
    )
    {
        destcont.erase(destfirst, destlast);
        return destcont.insert(
            destlast,
            std::forward<std::remove_reference_t<decltype(srcelem)>>(srcelem)
        );
    }

    template <typename T>
    static auto replace_from_to(
        std::list<T>& destcont, std::input_iterator auto destfirst, std::input_iterator auto destlast,
        std::list<T>&& srcrange
    )
    {
        destcont.erase(destfirst, destlast);
        destcont.splice(destlast, std::move(srcrange));
    }

    static size_t skipws(points_to_token_and_input auto& iter, const points_to_token_and_input auto end, bool skip_newlines = false)
    {
        size_t count = 0;
        while (iter != end &&
                (iter->kind == ::supdef::token_kind::horizontal_whitespace ||
                    (skip_newlines && iter->kind == ::supdef::token_kind::newline)))
        {
            stdranges::advance(iter, 1);
            count++;
        }
        return count;
    }

    template <typename IterT, typename PredT>
        requires points_to_token_and_input<IterT> && predicate<PredT, IterT>
    static size_t skip_until(IterT& iter, const IterT end, PredT pred)
    {
        size_t count = 0;
        if constexpr (std::invocable<PredT, IterT>)
        {
            while (iter != end && !pred(iter))
            {
                stdranges::advance(iter, 1);
                count++;
            }
        }
        else
        {
            while (iter != end && !pred(*iter))
            {
                stdranges::advance(iter, 1);
                count++;
            }
        }
        return count;
    }

    template <typename IterT>
        requires points_to_token_and_input<IterT>
    static size_t skip_until(IterT& iter, const IterT end, ::supdef::token_kind kind)
    {
        return skip_until(iter, end, [kind](const ::supdef::token& tok) {
            return tok.kind == kind;
        });
    }

    template <typename IterT, typename PredT, typename FnT>
        requires points_to_token_and_input<IterT> && predicate<PredT, IterT> && invokeable<FnT&&, IterT>
    static size_t skip_until(IterT& iter, const IterT end, PredT pred, FnT&& fn)
    {
        size_t count = 0;
        FnT fncpy = std::forward<FnT>(fn);
        if constexpr (std::invocable<PredT, IterT>)
        {
            while (iter != end && !pred(iter))
            {
                if constexpr (std::invocable<FnT&&, IterT>)
                    std::invoke(fncpy, iter);
                else
                    std::invoke(fncpy, *iter);
                stdranges::advance(iter, 1);
                count++;
            }
        }
        else
        {
            while (iter != end && !pred(*iter))
            {
                if constexpr (std::invocable<FnT&&, IterT>)
                    std::invoke(fncpy, iter);
                else
                    std::invoke(fncpy, *iter);
                stdranges::advance(iter, 1);
                count++;
            }
        }
        return count;
    }
    
    template <typename IterT, typename FnT>
        requires points_to_token_and_input<IterT> && invokeable<FnT&&, IterT>
    static size_t skip_until(IterT& iter, const IterT end, ::supdef::token_kind kind, FnT&& fn)
    {
        return skip_until(iter, end, [kind](const ::supdef::token& tok) {
            return tok.kind == kind;
        }, std::forward<FnT>(fn));
    }

    template <typename IterT, typename PredT>
        requires points_to_token_and_input<IterT> && predicate<PredT, IterT>
    static size_t skip_while(IterT& iter, const IterT end, PredT&& pred)
    {
        return skip_until(iter, end, std::not_fn(std::forward<PredT>(pred)));
    }

    template <typename IterT>
        requires points_to_token_and_input<IterT>
    static size_t skip_while(IterT& iter, const IterT end, ::supdef::token_kind kind)
    {
        return skip_until(iter, end, [kind](const ::supdef::token& tok) {
            return tok.kind != kind;
        });
    }

    template <typename IterT, typename PredT, typename FnT>
        requires points_to_token_and_input<IterT> && predicate<PredT, IterT> && invokeable<FnT&&, IterT>
    static size_t skip_while(IterT& iter, const IterT end, PredT&& pred, FnT&& fn)
    {
        return skip_until(iter, end, std::not_fn(std::forward<PredT>(pred)), std::forward<FnT>(fn));
    }

    template <typename IterT, typename FnT>
        requires points_to_token_and_input<IterT> && invokeable<FnT&&, IterT>
    static size_t skip_while(IterT& iter, const IterT end, ::supdef::token_kind kind, FnT&& fn)
    {
        return skip_until(iter, end, [kind](const ::supdef::token& tok) {
            return tok.kind != kind;
        }, std::forward<FnT>(fn));
    }
}

#if 0
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
        (sv.size() == 1 && ::supdef::unicat::is_digit(sv.front()) && ::supdef::unicode::numeric_value<long long>(sv.front()) == 0) ||
        strmatch(sv, U"false"sv, false) || strmatch(sv, U"no"sv, false) || strmatch(sv, U"off"sv, false)
    )
        return false;
    if (
        (sv.size() == 1 && ::supdef::unicat::is_digit(sv.front()) && ::supdef::unicode::numeric_value<long long>(sv.front()) != 0) ||
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
#endif

namespace detail
{
    namespace runnable_langs
    {
        static constexpr auto _get_all_langs()
        {
#if defined(__INTELLISENSE__)
            constexpr std::array<std::tuple<std::u32string_view, ::supdef::parser::registered_runnable::lang::identifier>, 0> langs = {};
#else
            constexpr std::array basic_langs_names  = magic_enum::enum_names <::supdef::parser::registered_runnable::lang::identifier>();
            constexpr std::array basic_langs_values = magic_enum::enum_values<::supdef::parser::registered_runnable::lang::identifier>();
            static_assert(basic_langs_names.size() == basic_langs_values.size());
            static_assert(basic_langs_names.size() == ::supdef::parser::registered_runnable::lang::_langidentcount);

            constexpr std::tuple aliases_langs = std::make_tuple(
                std::make_tuple (
                    std::string_view{ "c++" },
                    ::supdef::parser::registered_runnable::lang::identifier::cpp
                ),
                std::make_tuple (
                    std::string_view{ "cxx" },
                    ::supdef::parser::registered_runnable::lang::identifier::cpp
                ),
                std::make_tuple (
                    std::string_view{ "c#" },
                    ::supdef::parser::registered_runnable::lang::identifier::csharp
                ),
                std::make_tuple (
                    std::string_view{ "f#" },
                    ::supdef::parser::registered_runnable::lang::identifier::fsharp
                )
            );

            constexpr std::tuple all_langs = std::tuple_cat(
                zip(
                    as_tuple(basic_langs_names), as_tuple(basic_langs_values)
                ),
                aliases_langs
            );

            constexpr std::array langs = mk_array<
                std::tuple<
                    std::string_view,
                    ::supdef::parser::registered_runnable::lang::identifier
                >
            >(all_langs);
#endif
            return langs;
        }
    }
}

constexpr std::optional<supdef::parser::registered_runnable::lang::identifier>
supdef::parser::registered_runnable::is_lang_identifier(std::u32string_view sv)
{
    constexpr std::array langs = ::detail::runnable_langs::_get_all_langs();
    
    std::bitset<langs.size()> matches;
    static_assert(std::indirectly_writable<bitset_inserter<langs.size()>, bool>);

    stdranges::transform(
        langs, bitset_inserter{matches},
        // op
        [&sv](const ascii_to_char32_view& langident) {
            return strmatch(sv, langident, false);
        },
        // projection of range `langs`
        [](const std::tuple<std::string_view, ::supdef::parser::registered_runnable::lang::identifier>& lang) {
            return ascii_to_char32_view{ std::get<0>(lang) };
        }
    );
    if (matches.none())
        return std::nullopt;
    size_t longest = 0, longest_idx = 0;
    for (size_t i = 0; i < langs.size(); i++)
    {
        if (matches._Unchecked_test(i) && std::get<0>(langs[i]).size() > longest)
        {
            longest = std::get<0>(langs[i]).size();
            longest_idx = i;
        }
    }
    return std::get<1>(langs[longest_idx]);
}

#if 0
#undef  __MK_ASSERT
#undef  __MK_UNASSERT
#define __MK_ASSERT(xstr, xval) static_assert( \
    ::supdef::parser::registered_runnable::is_lang_identifier(U ## xstr).has_value() && \
    ::supdef::parser::registered_runnable::is_lang_identifier(U ## xstr).value() == ::supdef::parser::registered_runnable::lang::identifier::xval, \
    #xstr " should be a valid language identifier" \
)
#define __MK_UNASSERT(xstr) static_assert( \
    !::supdef::parser::registered_runnable::is_lang_identifier(U ## xstr).has_value(), \
    #xstr " should not be a valid language identifier" \
)

__MK_ASSERT("c", c);
__MK_ASSERT("cpp", cpp);
__MK_ASSERT("cxx", cpp);
__MK_ASSERT("c++", cpp);
__MK_ASSERT("rust", rust);
__MK_ASSERT("d", d);
__MK_ASSERT("zig", zig);
__MK_ASSERT("csharp", csharp);
__MK_ASSERT("c#", csharp);
__MK_ASSERT("fsharp", fsharp);
__MK_ASSERT("f#", fsharp);
__MK_ASSERT("java", java);

__MK_UNASSERT("c#sharp");
__MK_UNASSERT("f#sharp");
__MK_UNASSERT("fsharp#");

#undef __MK_ASSERT
#undef __MK_UNASSERT
#endif

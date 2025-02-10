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

    if (
        sv.front() == U'0' ||
        sv.starts_with(U"false"sv) || sv.starts_with(U"no"sv) || sv.starts_with(U"off"sv) ||
        sv.starts_with(U"FALSE"sv) || sv.starts_with(U"NO"sv) || sv.starts_with(U"OFF"sv))
        return false;
    if (
        sv.front() == U'1' ||
        sv.starts_with(U"true"sv) || sv.starts_with(U"yes"sv) || sv.starts_with(U"on"sv) ||
        sv.starts_with(U"TRUE"sv) || sv.starts_with(U"YES"sv) || sv.starts_with(U"ON"sv))
        return true;
    return boost::logic::indeterminate;
}
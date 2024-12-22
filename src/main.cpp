#include <types.hpp>
#include <parser.hpp>
#include <file.hpp>
#include <init.hpp>
#include <unicode.hpp>
#include <iostream>

#include <unicode/ustream.h>
#include <unicode/regex.h>

namespace stdfs = std::filesystem;

extern bool matches_import(const icu::UnicodeString& str);

int main(int argc, char const *argv[])
{
    supdef::global_initializers();
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    std::error_code ec;
    stdfs::path sourcefilepath = stdfs::canonical(argv[1], ec);
    if (ec)
    {
        std::cerr << "failed to get canonical path: " << ec.message() << '\n';
        return 1;
    }

    supdef::source_file file(sourcefilepath);
    std::cout << "File: "  << file.filename() << '\n';
    std::cout << "Data:\n" << file.data() << '\n';

    std::array<icu::UnicodeString, 5> import_test_strings = {
        _("@import \"test1\""),
        _("  @                 import \"test2\"            "),
        _("  @                 import \"test2\"            \n"),
        _("  @                 import\"test2\"            "),
        _("  import \"test2\"")
    };
    std::cout << "Import test:\n";
    for (const auto& str : import_test_strings)
    {
        std::cout << "String: " << str << '\n';
        std::cout << "Matches: " << matches_import(str) << '\n';
    }

    return 0;
}

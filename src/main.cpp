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

    supdef::parser parser(sourcefilepath);
    parser.output_to("test-original.txt");
    parser.do_stage1();
    parser.output_to("test-stage1.txt");
    parser.do_stage2();
    parser.output_to("test-stage2.txt");
    parser.do_stage3();
    parser.output_to("test-stage3.txt");
    parser.output_to("test-stage3-tokens.txt", supdef::parser::output_kind::tokens);

    return 0;
}

#include <version.hpp>

#include <types.hpp>
#include <parser.hpp>
#include <file.hpp>
#include <init.hpp>
#include <unicode.hpp>
#include <detail/globals.hpp>

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include <memory>
#include <algorithm>
#include <iterator>
#include <numeric>

#include <sys/ioctl.h>
#include <unistd.h>

#include <unicode/ustream.h>
#include <unicode/regex.h>

#include <boost/pfr.hpp>

#include <magic_enum.hpp>
#include <magic_enum_flags.hpp>

#include <argparse/argparse.hpp>

#include <boost/preprocessor/config/config.hpp>
#include <boost/preprocessor/limits.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/stringize.hpp>

GLOBAL_GETTER_DECL(std::vector<stdfs::path>, supdef_import_paths)

struct cmdline
{
    stdfs::path input_file;
    stdfs::path output_file;
    std::string progname;
    unsigned stop_after_stage;
#if 1
    ::supdef::parser::output_kind output_kind;
#else
    std::underlying_type_t<output> output_kind;
#endif
    int verbosity;
};

static ::cmdline supdef_cmdline{
    .input_file = "",
    .output_file = "",
    .progname = "",
    .stop_after_stage = 3U,
    .output_kind = ::supdef::parser::output_kind::text,
    .verbosity = 0
};

static consteval std::string consteval_stringize(unsigned i)
{
    switch (i)
    {
#undef  __MACRO
#define __MACRO(z, n, data) case n: return BOOST_PP_STRINGIZE(n);

#ifndef __INTELLISENSE__
    BOOST_PP_REPEAT_FROM_TO(0, BOOST_PP_LIMIT_MAG, __MACRO, ~);
#else
    BOOST_PP_REPEAT_FROM_TO(0, 10, __MACRO, ~);
#endif
#undef  __MACRO
    default: return "???";
    }
}

static consteval std::string generate_stage_numbers_string()
{
    std::string ret;
    for (unsigned i = 1; i < SUPDEF_MAX_PARSING_PHASE; ++i)
    {
        if (i > 1)
            ret += ", ";
        ret += consteval_stringize(i);
    }
    ret += " or " + consteval_stringize(SUPDEF_MAX_PARSING_PHASE);
    return ret;
}

constexpr std::string SUPDEF_STAGE_NUMBERS = generate_stage_numbers_string();

static void parse_cmdline(int argc, char const* const* argv)
{
#if 0
    supdef_cmdline.progname = argv[0];
#else
    supdef_cmdline.progname = program_invocation_short_name;
#endif

    argparse::ArgumentParser progargs(supdef_cmdline.progname, SUPDEF_VERSION_STRING, argparse::default_arguments::help);
    progargs.add_description("supdef - a simple preprocessor for (and written in) C/C++");

    progargs.add_argument("-V", "--version")
        .help("print version info and exit")
        .action([](...) noexcept -> void {
            std::cout << "supdef version " << SUPDEF_VERSION_STRING << '\n';
            ::exit(EXIT_SUCCESS);
        })
        .append()
        .flag()
        .nargs(0)
        ;
    progargs.add_argument("-I", "--import")
        .help("add a directory to the import search path")
        .action([](const std::string& value) noexcept -> void {
            try
            {    
                ::supdef::globals::get_supdef_import_paths().push_back(
                    stdfs::canonical(value)
                );
            }
            catch (const stdfs::filesystem_error& e)
            {
                using namespace std::string_literals;
                supdef::printer::fatal(
                    "failed to add import path `"s + e.path1().string() + "`: "s +
                    e.code().message()
                );
                ::exit(EXIT_FAILURE);
            }
            catch (const std::exception& e)
            {
                using namespace std::string_literals;
                supdef::printer::fatal(
                    "failed to add import path `"s + value + "`: "s +
                    e.what()
                );
                ::exit(EXIT_FAILURE);
            }
            catch (...)
            {
                using namespace std::string_literals;
                supdef::printer::fatal(
                    "failed to add import path `"s + value + "`: unknown error"
                );
                ::exit(EXIT_FAILURE);
            }
        })
        .nargs(1)
        .metavar("<import-path>")
        ;
    progargs.add_argument("-v", "--verbose")
        .help("increase verbosity")
        .action([&](...) noexcept -> void { ++supdef_cmdline.verbosity; })
        .append()
        .flag()
        .nargs(0)
        ;
    progargs.add_argument("-q", "--quiet")
        .help("decrease verbosity")
        .action([&](...) noexcept -> void { --supdef_cmdline.verbosity; })
        .append()
        .flag()
        .nargs(0)
        ;
    progargs.add_argument("-s", "--stage")
        .help("output processed content after specified stage")
        .nargs(1)
        .scan<'u', unsigned>()
        .metavar("<stage-number>")
        ;
    progargs.add_argument("--output-tokens")
        .help("output tokenized content")
        .flag()
        .nargs(0)
        ;
    progargs.add_argument("--output-ast")
        .help("output ast")
        .flag()
        .nargs(0)
        ;
    progargs.add_argument("--output-imports")
        .help("output imported files")
        .flag()
        .nargs(0)
        ;
    progargs.add_argument("--output-original")
        .help("output original content")
        .flag()
        .nargs(0)
        ;
    progargs.add_argument("-a", "--all")
        .help("output all")
        .flag()
        .nargs(0)
        ;
    progargs.add_argument("-o", "--output-file")
        .help("output file")
        .nargs(1)
        .required()
        .metavar("<output-file>")
        ;
    progargs.add_argument("input-file")
        .help("input file")
        .nargs(1)
        .metavar("<input-file>")
        ;

    try
    {
        progargs.parse_args(argc, argv);
    }
    catch (const std::runtime_error& e)
    {
        using namespace std::string_literals;
        supdef::printer::fatal("failed to parse command line arguments: "s + e.what());
        std::cerr << progargs;
        ::exit(EXIT_FAILURE);
    }

    // required arguments
    supdef_cmdline.output_file = progargs.get<std::string>("-o");
    supdef_cmdline.input_file = progargs.get<std::string>("input-file");

    // optional arguments
    if (progargs.is_used("-s"))
    {
        supdef_cmdline.stop_after_stage = progargs.get<unsigned>("-s");
        if (supdef_cmdline.stop_after_stage > SUPDEF_MAX_PARSING_PHASE || supdef_cmdline.stop_after_stage == 0U)
        {
            using namespace std::string_literals;
            supdef::printer::fatal(
                "invalid stage number: "s + std::to_string(supdef_cmdline.stop_after_stage) +
                " (must be "+ SUPDEF_STAGE_NUMBERS + ")"
            );
            ::exit(EXIT_FAILURE);
        }
    }
    else
        supdef_cmdline.stop_after_stage = SUPDEF_MAX_PARSING_PHASE;

    if (progargs.is_used("--output-tokens"))
        supdef_cmdline.output_kind = ::supdef::parser::output_kind(
            supdef_cmdline.output_kind | ::supdef::parser::output_kind::tokens
        );
    if (progargs.is_used("--output-ast"))
        supdef_cmdline.output_kind = ::supdef::parser::output_kind(
            supdef_cmdline.output_kind | ::supdef::parser::output_kind::ast
        );
    if (progargs.is_used("--output-imports"))
        supdef_cmdline.output_kind = ::supdef::parser::output_kind(
            supdef_cmdline.output_kind | ::supdef::parser::output_kind::imports
        );
    if (progargs.is_used("--output-original"))
        supdef_cmdline.output_kind = ::supdef::parser::output_kind(
            supdef_cmdline.output_kind | ::supdef::parser::output_kind::original
        );
    if (progargs.is_used("-a"))
        supdef_cmdline.output_kind = ::supdef::parser::output_kind(
            supdef_cmdline.output_kind | ::supdef::parser::output_kind::all
        );
}


static int old_main(int argc, char const* argv[]);
static int real_main(int argc, char const* argv[]);

int main(int argc, char const *argv[])
{
    supdef::global_initializers();
#if 0
    old_main(argc, argv);
#else
    real_main(argc, argv);
#endif
    return 0;
}

static int old_main(int argc, char const* argv[])
{
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

static int real_main(int argc, char const* argv[])
{
    parse_cmdline(argc, argv);

    if (supdef_cmdline.verbosity > 2)
    {
        std::array field_names = boost::pfr::names_as_array<cmdline>();
        size_t field_counter = 0;
        boost::pfr::for_each_field(
            supdef_cmdline,
            [&field_names, &field_counter](const auto& field)
            {
                if constexpr (std::is_enum_v<std::decay_t<decltype(field)>>)
                    supdef::printer::info(std::string(field_names[field_counter].data()) + ": " + magic_enum::enum_flags_name(field));
                else if constexpr (requires { { field.to_string() } -> std::convertible_to<std::string>; })
                    supdef::printer::info(std::string(field_names[field_counter].data()) + ": " + field.to_string());
                else if constexpr (requires { { std::to_string(field) } -> std::convertible_to<std::string>; })
                    supdef::printer::info(std::string(field_names[field_counter].data()) + ": " + std::to_string(field));
                else if constexpr (requires { { field.string() } -> std::convertible_to<std::string>; })
                    supdef::printer::info(std::string(field_names[field_counter].data()) + ": " + field.string());
                else
                    supdef::printer::info(std::string(field_names[field_counter].data()) + ": " + field);
                ++field_counter;
            }
        );
    }

    std::error_code ec;
    stdfs::path sourcefilepath = stdfs::canonical(supdef_cmdline.input_file, ec);
    if (ec)
    {
        supdef::printer::fatal("failed to get canonical path: " + ec.message());
        return 1;
    }

    supdef::parser parser(sourcefilepath);
    parser.do_stage1();
    if (supdef_cmdline.stop_after_stage == 1U)
    {
        parser.output_to(supdef_cmdline.output_file, supdef_cmdline.output_kind);
        return 0;
    }
    parser.do_stage2();
    if (supdef_cmdline.stop_after_stage == 2U)
    {
        parser.output_to(supdef_cmdline.output_file, supdef_cmdline.output_kind);
        return 0;
    }
    parser.do_stage3();
    if (supdef_cmdline.stop_after_stage == 3U)
    {
        parser.output_to(supdef_cmdline.output_file, supdef_cmdline.output_kind);
        return 0;
    }
    parser.do_stage4();
    parser.output_to(supdef_cmdline.output_file, supdef_cmdline.output_kind);
    return 0;
}

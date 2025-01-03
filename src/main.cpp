#include <version.hpp>

#include <types.hpp>
#include <parser.hpp>
#include <file.hpp>
#include <init.hpp>
#include <unicode.hpp>

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

#include <argparse/argparse.hpp>

namespace stdfs = std::filesystem;

#if 0
static int get_term_line_length(void)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

static decltype(auto) positional_opts(void)
{
    namespace po = boost::program_options;
    
    static po::positional_options_description positionals;
    static po::options_description corresp("options corresponding to positional arguments", get_term_line_length());
    
    static std::once_flag once;
    std::call_once(once, []{
        positionals.add("input-file", 1);
        corresp.add_options()
            ("input-file", po::value<std::string>(), "input file")
            ;
    });
    
    std::pair ret{ positionals, corresp };
    return ret;
}

static decltype(auto) misc_opts(void)
{
    namespace po = boost::program_options;
    
    static po::options_description misc("miscellaneous options", get_term_line_length());
    
    static std::once_flag once;
    std::call_once(once, []{
        misc.add_options()
            ("help,h", "produce help message")
            ("version,v", "print version")
            ;
    });
    
    return misc;
}

static decltype(auto) stage_opts(void)
{
    namespace po = boost::program_options;
    
    static po::options_description stages("stage options", get_term_line_length());
    
    static std::once_flag once;
    std::call_once(once, []{
        stages.add_options()
            ("stage1", "run stage 1")
            ("stage2", "run stage 2")
            ("stage3", "run stage 3")
            ("all", "run all stages")
            ;
    });
    
    return stages;
}

static decltype(auto) output_opts(void)
{
    namespace po = boost::program_options;
    
    static po::options_description output("output options", get_term_line_length());
    
    static std::once_flag once;
    std::call_once(once, []{
        output.add_options()
            ("output-file,o", po::value<std::string>()->default_value("output.txt"), "output file")
            ("tokens", "output tokens")
            ("ast", "output ast")
            ;
    });
    
    return output;
}

static decltype(auto) parse_cmdline(int argc, char const* const* argv)
{
    namespace po = boost::program_options;
    
    po::options_description printed_opts("supdef options", get_term_line_length());
    printed_opts.add(misc_opts());
    printed_opts.add(stage_opts());
    printed_opts.add(output_opts());
    
    std::pair positionals(positional_opts());
    po::options_description all_opts(printed_opts);
    all_opts.add(positionals.second);

    auto usage =
        [&printed_opts, &positionals](std::ostream& os, const char* progname = "supdef") -> void
        {
            using namespace std::string_literals;

            std::string usage_prefix = "usage: "s + progname + " [supdef options] ";

            const std::vector<boost::shared_ptr<po::option_description>>& opts = positionals.second.options();
            unsigned positional_count = opts.size();
            std::vector<std::string> explanations(positional_count);
            size_t max_len = 0;

            for (unsigned i = 0; i < positional_count; ++i)
            {
                const std::string& opt_name = opts.at(i)->long_name();

                if (i > 0)
                    usage_prefix.push_back(' ');
                usage_prefix.append("<"s + opt_name + ">"s);

                explanations[i].append(2, ' ');
                explanations[i].append("<"s + opt_name + ">"s);

                max_len = std::max(max_len, explanations[i].size());
            }

            for (unsigned i = 0; i < positional_count; ++i)
            {
                const std::string& opt_expl = opts.at(i)->description();

                explanations[i].append(max_len - explanations[i].size(), ' ');
                explanations[i].append("  "s + opt_expl);
            }
            usage_prefix.append("\n\n");
            usage_prefix.append("positional arguments:\n");
            usage_prefix.append(
                std::accumulate(
                    explanations.begin(), explanations.end(), ""s,
                    [](const std::string& acc, const std::string& s) { return acc + s + '\n'; }
                )
            );
            os << usage_prefix << '\n';
            os << printed_opts << '\n';
        };
        
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(all_opts).positional(positionals.first).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        usage(std::cout, argv[0]);
        ::exit(EXIT_SUCCESS);
    }
    if (vm.count("version"))
    {
        std::cout << "supdef version " << SUPDEF_VERSION_STRING << "\n";
        ::exit(EXIT_SUCCESS);
    }

    if (vm.count("input-file") != 1)
    {
        std::cerr << "no input file specified\n";
        usage(std::cerr, argv[0]);
        ::exit(EXIT_FAILURE);
    }

    return vm;
}

#else

static argparse::ArgumentParser build_cmdline_parser(int argc, char const* const* argv)
{
    argparse::ArgumentParser argparser("supdef");
    argparser.add_argument("input-file")
        .help("input file")
        .nargs(1)
        ;
    argparser.add_argument("-h", "--help")
        .help("produce help message")
        .default_value(false)
        .implicit_value(true)
        ;
    argparser.add_argument("-v", "--version")
        .help("print version")
        .default_value(false)
        .implicit_value(true)
        ;
    argparser.add_argument("--stage1")
        .help("run stage 1")
        .default_value(false)
        .implicit_value(true)
        ;
    argparser.add_argument("--stage2")
        .help("run stage 2")
        .default_value(false)
        .implicit_value(true)
        ;
    argparser.add_argument("--stage3")
        .help("run stage 3")
        .default_value(false)
        .implicit_value(true)
        ;
    argparser.add_argument("--all")
        .help("run all stages")
        .default_value(false)
        .implicit_value(true)
        ;
    argparser.add_argument("-o", "--output-file")
        .help("output file")
        .default_value("output.txt")
        ;
    argparser.add_argument("--tokens")
        .help("output tokens")
        .default_value(false)
        .implicit_value(true)
        ;
    argparser.add_argument("--ast")
        .help("output ast")
        .default_value(false)
        .implicit_value(true)
        ;
    return argparser;
}
#endif

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
    argparse::ArgumentParser argparser = build_cmdline_parser(argc, argv);
    std::error_code ec;
    stdfs::path sourcefilepath = stdfs::canonical(// TODO
    "");
    if (ec)
    {
        std::cerr << "failed to get canonical path: " << ec.message() << '\n';
        return 1;
    }

    supdef::parser parser(sourcefilepath);

    return 0;
}

#ifndef PARSER_HPP
#define PARSER_HPP

#include <types.hpp>
#include <directives.hpp>
#include <file.hpp>

#include <filesystem>
#include <vector>

#include <unicode/ustream.h>
#include <unicode/unistr.h>

namespace supdef
{
    class parser
    {
    public:
        parser(std::filesystem::path filename);
        ~parser();

    private:
        source_file m_file;
        location m_loc;
    };
}

#endif

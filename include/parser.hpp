#ifndef PARSER_HPP
#define PARSER_HPP

#include <types.hpp>
#include <directives.hpp>
#include <mmap.hpp>

#include <filesystem>
#include <vector>

namespace supdef
{
    std::vector<uint_least32_t> read_whole_file(const std::filesystem::path& filename);

    class parser
    {
    public:
        parser(const char* filename);
        ~parser();

    private:
        mmaped_file m_file;
        location m_loc;
        /* token_tree m_tree; */
    };
}

#endif

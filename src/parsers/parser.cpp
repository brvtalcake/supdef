#include <file.hpp>
#include <unicode.hpp>
#include <parser.hpp>

supdef::parser::parser(std::filesystem::path filename)
    : m_file(filename), m_loc()
{
}

supdef::parser::~parser()
{
}
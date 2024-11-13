#include <types.hpp>
#include <parser.hpp>

#include <iostream>

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    std::vector<uint_least32_t> codepoints = supdef::read_whole_file(argv[1]);

    std::cout << "Codepoints:\n" << std::hex << std::showbase;
    for (auto&& codepoint : codepoints)
        std::cout << codepoint << ' ';

    return 0;
}

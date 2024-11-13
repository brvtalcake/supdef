#include <parser.hpp>
#include <mmap.hpp>

extern "C" {
#include <grapheme.h>
}

std::vector<uint_least32_t> supdef::read_whole_file(const std::filesystem::path& filename)
{
    std::vector<uint_least32_t> ret;
    mmaped_file file(filename);
    
    const char* data = file.data();
    size_t size = file.size();
    
    ret.reserve(size);
    for (size_t i = 0; i < size;)
    {
        uint_least32_t codepoint;
        size_t len = grapheme_decode_utf8(data + i, size - i, &codepoint);
        if (len == 0 || codepoint == GRAPHEME_INVALID_CODEPOINT)
            throw std::runtime_error("Failed to decode codepoint");
        
        ret.push_back(codepoint);
        i += len;
    }
    
    return ret;
}
#include <experimental/scope>

#include <parser.hpp>
#include <file.hpp>
#include <unicode.hpp>

extern "C" {
#include <grapheme.h>
}

#include <unicode/normalizer2.h>

namespace stdx = std::experimental;
/* std::vector<uint_least32_t> supdef::read_whole_file(const std::filesystem::path& filename)
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
} */

supdef::source_file::source_file(const std::filesystem::path& filename)
    : m_data(), m_filename(filename)
{
    int fd = ::open(filename.c_str(), O_RDONLY);
    if (fd == -1)
        throw std::runtime_error("failed to open file");

    bool closedfd = false;
    stdx::scope_exit close_fd(
        [&fd, &closedfd] {
            if (closedfd)
                return;
            ::close(fd);
            closedfd = true;
        }
    );
    file_lock flock{fd, false};

    struct ::stat st;
    if (::fstat(fd, &st) == -1)
        throw std::runtime_error("failed to get file size");

    char* fbuf = static_cast<char*>(alloca(st.st_size + 1));
    ssize_t len = ::read(fd, fbuf, st.st_size);
    if (len == -1)
        throw std::runtime_error("failed to read file");
    fbuf[len] = '\0';

    flock.release();

    // TODO: add support for more encoding types
    m_data = icu::UnicodeString::fromUTF8(fbuf);

    if (m_data.isBogus())
        throw std::runtime_error("failed to convert file to UnicodeString");
    
    m_data = uninorm::normalize(m_data);
}

supdef::source_file::~source_file()
{
    // nothing to do
}

const icu::UnicodeString& supdef::source_file::data() const noexcept
{
    return m_data;
}

const std::filesystem::path& supdef::source_file::filename() const noexcept
{
    return m_filename;
}
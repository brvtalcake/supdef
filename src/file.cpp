#include <experimental/scope>

#include <parser.hpp>
#include <file.hpp>
#include <unicode.hpp>

#include <simdutf.h>

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

::supdef::source_file::source_file(const stdfs::path& filename)
    : m_orig(), m_data(), m_filename(std::make_shared<const stdfs::path>(filename))
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

    /* m_orig = simdutf::convert_utf8_to_utf32(fbuf, len); */
    // TODO: add support for more encoding types
    // TODO: could probably also use std::wbuffer_convert
    //char32_t* buf = new char32_t[len];
    std::unique_ptr<char32_t[]> buf(new char32_t[len]);
    size_t len32 = simdutf::convert_utf8_to_utf32(fbuf, len, buf.get());
    if (len32 == 0)
        throw std::runtime_error("failed to convert file to utf32, or empty file");

    m_orig = std::u32string(buf.get(), len32);
    m_data = m_orig;
}

::supdef::source_file::source_file(stdfs::path&& filename)
    : m_orig(), m_data(), m_filename(std::make_shared<const stdfs::path>(std::move(filename)))
{
    int fd = ::open(m_filename->c_str(), O_RDONLY);
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

    /* m_orig = simdutf::convert_utf8_to_utf32(fbuf, len); */
    // TODO: add support for more encoding types
    // TODO: could probably also use std::wbuffer_convert
    //char32_t* buf = new char32_t[len];
    std::unique_ptr<char32_t[]> buf(new char32_t[len]);
    size_t len32 = simdutf::convert_utf8_to_utf32(fbuf, len, buf.get());
    if (len32 == 0)
        throw std::runtime_error("failed to convert file to utf32, or empty file");
    
    m_orig = std::u32string(buf.get(), len32);
    m_data = m_orig;
}

supdef::source_file::~source_file()
{
    // nothing to do a priori
}

std::u32string& supdef::source_file::data() noexcept
{
    return m_data;
}

const std::u32string& supdef::source_file::original_data() const noexcept
{
    return m_orig;
}

std::shared_ptr<const stdfs::path> supdef::source_file::filename() const noexcept
{
    return m_filename;
}
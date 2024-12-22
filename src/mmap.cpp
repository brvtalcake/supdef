#include <file.hpp>

#include <stdexcept>
#include <filesystem>
#include <climits>
#include <string>
#include <sstream>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

static_assert(sizeof(char) == sizeof(char), "char is not the same size as char");

supdef::mmaped_file::mmaped_file(const char* filename)
    : m_flock(), m_data(nullptr), m_size(0), m_fd(-1)
{
    m_fd = ::open(filename, O_RDONLY);
    if (m_fd == -1)
        throw std::runtime_error("failed to open file");
    
    m_flock = file_lock(m_fd, false);

    struct ::stat st;
    if (::fstat(m_fd, &st) == -1)
    {
        ::close(m_fd);
        throw std::runtime_error("failed to get file size");
    }

    m_size = st.st_size;
    m_data = static_cast<const char*>(::mmap(nullptr, m_size, PROT_READ, MAP_PRIVATE, m_fd, 0));
    if (m_data == MAP_FAILED)
    {
        ::close(m_fd);
        throw std::runtime_error("failed to map file");
    }
}

supdef::mmaped_file::mmaped_file(const std::filesystem::path& filename)
    : mmaped_file(filename.c_str())
{ }

supdef::mmaped_file::~mmaped_file()
{
    m_flock.release();
    ::munmap(const_cast<char*>(m_data), m_size);
    ::close(m_fd);
}

const char* supdef::mmaped_file::data() const noexcept
{
    return m_data;
}

size_t supdef::mmaped_file::size() const noexcept
{
    return m_size;
}

supdef::mmaped_file::operator std::istringstream() const
{
    return std::istringstream{
        std::string(m_data, m_size), std::ios_base::in
    };
}

namespace
{
    template <typename T>
    consteval T __select(T obj)
    {
        return obj;
    }
}
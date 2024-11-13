#include <mmap.hpp>

#include <stdexcept>
#include <filesystem>
#include <climits>

static_assert(sizeof(char) == sizeof(char), "char is not the same size as char");

supdef::mmaped_file::mmaped_file(const char* filename)
{
#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
    m_fd = ::open(filename, O_RDONLY);
    if (m_fd == -1)
        throw std::runtime_error("Failed to open file");

    struct ::stat st;
    if (::fstat(m_fd, &st) == -1)
    {
        ::close(m_fd);
        throw std::runtime_error("Failed to get file size");
    }

    m_size = st.st_size;
    m_data = static_cast<const char*>(::mmap(nullptr, m_size, PROT_READ, MAP_PRIVATE, m_fd, 0));
    if (m_data == MAP_FAILED)
    {
        ::close(m_fd);
        throw std::runtime_error("Failed to map file");
    }
#elif defined(_WIN32)
    m_file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (m_file == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Failed to open file");

    m_mapping = CreateFileMappingA(m_file, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (m_mapping == nullptr)
    {
        CloseHandle(m_file);
        throw std::runtime_error("Failed to create file mapping");
    }

    m_size = GetFileSize(m_file, nullptr);
    m_data = static_cast<const char*>(MapViewOfFile(m_mapping, FILE_MAP_READ, 0, 0, 0));
    if (m_data == nullptr)
    {
        CloseHandle(m_mapping);
        CloseHandle(m_file);
        throw std::runtime_error("Failed to map file");
    }
#endif
}

// TODO: Rewrite suspicious wchar_t --> char conversion
supdef::mmaped_file::mmaped_file(const wchar_t* filename)
{
#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
    char real_filename[PATH_MAX];
    if (::wcstombs(real_filename, filename, PATH_MAX) == -1)
        throw std::runtime_error("Failed to convert filename");

    m_fd = ::open(real_filename, O_RDONLY);
    if (m_fd == -1)
        throw std::runtime_error("Failed to open file");

    struct ::stat st;
    if (::fstat(m_fd, &st) == -1)
    {
        ::close(m_fd);
        throw std::runtime_error("Failed to get file size");
    }

    m_size = st.st_size;
    m_data = static_cast<const char*>(::mmap(nullptr, m_size, PROT_READ, MAP_PRIVATE, m_fd, 0));
    if (m_data == MAP_FAILED)
    {
        ::close(m_fd);
        throw std::runtime_error("Failed to map file");
    }
#elif defined(_WIN32)
    m_file = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (m_file == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Failed to open file");

    m_mapping = CreateFileMappingW(m_file, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (m_mapping == nullptr)
    {
        CloseHandle(m_file);
        throw std::runtime_error("Failed to create file mapping");
    }

    m_size = GetFileSize(m_file, nullptr);
    m_data = static_cast<const char*>(MapViewOfFile(m_mapping, FILE_MAP_READ, 0, 0, 0));
    if (m_data == nullptr)
    {
        CloseHandle(m_mapping);
        CloseHandle(m_file);
        throw std::runtime_error("Failed to map file");
    }
#endif
}

supdef::mmaped_file::mmaped_file(const std::filesystem::path& filename)
    : mmaped_file(filename.c_str())
{ }

supdef::mmaped_file::~mmaped_file()
{
#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
    ::munmap(const_cast<char*>(m_data), m_size);
    ::close(m_fd);
#elif defined(_WIN32)
    UnmapViewOfFile(m_data);
    CloseHandle(m_mapping);
    CloseHandle(m_file);
#endif
}

const char* supdef::mmaped_file::data() noexcept
{
    return m_data;
}

size_t supdef::mmaped_file::size() noexcept
{
    return m_size;
}
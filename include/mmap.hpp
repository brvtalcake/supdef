#ifndef MMAP_HPP
#define MMAP_HPP

#include <filesystem>
#include <cstdint>

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#error "Unsupported platform"
#endif

namespace supdef
{
    class mmaped_file
    {
    public:
        mmaped_file(const char* filename);
        mmaped_file(const wchar_t* filename);
        mmaped_file(const std::filesystem::path& filename);

        ~mmaped_file();
        const char* data() noexcept;
        size_t size() noexcept;
    
    private:
        const char* m_data;
        size_t m_size;
#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
        int m_fd;
#elif defined(_WIN32)
        HANDLE m_file;
        HANDLE m_mapping;
#endif
    };
}

#endif

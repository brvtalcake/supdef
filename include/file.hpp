#ifndef MMAP_HPP
#define MMAP_HPP

#include <types.hpp>

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
    class file_lock
    {
        static void lock(int fd, bool exclusive = true);
        static void unlock(int fd);
    public:
        file_lock();
        file_lock(int fd, bool exclusive = true);
        ~file_lock();
        void release();
    private:
        int m_fd;
        bool m_is_locked;
    };

    class mmaped_file
    {
    public:
        mmaped_file(const char* filename);
        mmaped_file(const std::filesystem::path& filename);

        ~mmaped_file();
        const char* data() const noexcept;
        size_t size() const noexcept;

        operator std::istringstream() const;
    
    private:
        file_lock m_flock;
        const char* m_data;
        size_t m_size;
        int m_fd;
    };

    class source_file
    {
    public:
        source_file(const stdfs::path& filename);
        source_file(stdfs::path&& filename);
        ~source_file();

        std::u32string& data() noexcept;
        const std::u32string& original_data() const noexcept;
        
        shared_ptr<const stdfs::path> filename() const noexcept;
    private:
        std::u32string m_orig;
        std::u32string m_data;
        shared_ptr<const stdfs::path> m_filename;
    };
}

#endif

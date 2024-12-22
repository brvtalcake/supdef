#include <file.hpp>

/*

In file.hpp:
class file_lock
{
    static void lock(int fd, bool exclusive = true);
    static void unlock(int fd);
public:
    file_lock(int fd);
    ~file_lock();
    void release();
private:
    int m_fd;
    bool m_is_locked;
};

*/

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <climits>

namespace
{
    // FIXME: doesn't work on macOS
    std::string get_file_name(int fd)
    {
        const std::string procfilename = "/proc/self/fd/" + std::to_string(fd);
        char buf[PATH_MAX];

        ssize_t len = ::readlink(procfilename.c_str(), buf, sizeof(buf));

        if (len == -1)
            return procfilename;
        return std::string(buf, len);
    }
}

void supdef::file_lock::lock(int fd, bool exclusive)
{
    if (::flock(fd, exclusive ? LOCK_EX : LOCK_SH) == -1)
    {
        std::string file_name = get_file_name(fd);
        throw std::runtime_error("failed to lock file " + file_name + ": " + strerror(errno));
    }
}

void supdef::file_lock::unlock(int fd)
{
    if (::flock(fd, LOCK_UN) == -1)
    {
        std::string file_name = get_file_name(fd);
        throw std::runtime_error("failed to unlock file " + file_name + ": " + strerror(errno));
    }
}

supdef::file_lock::file_lock()
    : m_fd(-1), m_is_locked(false)
{ }

supdef::file_lock::file_lock(int fd, bool exclusive)
    : m_fd(fd), m_is_locked(true)
{
    lock(m_fd, exclusive);
}

supdef::file_lock::~file_lock()
{
    this->release();
}

void supdef::file_lock::release()
{
    if (m_is_locked)
    {
        unlock(m_fd);
        m_is_locked = false;
    }
}
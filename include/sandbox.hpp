#ifndef SANDBOX_HPP
#define SANDBOX_HPP

#include <linux/landlock.h>
#include <linux/cgroupstats.h>
#include <cap-ng.h>
#include <seccomp.h>

/* #include <process.hpp> */

namespace supdef
{
    namespace sandbox
    {
        enum type : unsigned
        {
            none     = 0,
            chroot   = 1U << 0,
            libcap   = 1U << 1,
            cgroup   = 1U << 2,
            seccomp  = 1U << 3,
            apparmor = 1U << 4,
            selinux  = 1U << 5,
            landlock = 1U << 6,
            all = chroot
                | libcap
                | cgroup
                | seccomp
                | apparmor
                | selinux
                | landlock
        };

        class directory
        {
        };
    }
}
#endif

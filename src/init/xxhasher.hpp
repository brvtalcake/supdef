#include <detail/globals.hpp>
#include <init.hpp>
#include <types.hpp>

static constexpr uint64_t seed_with_time()
{
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
    return std::bit_cast<uint64_t>(value.count());
}




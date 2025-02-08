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


GLOBAL_XXHASHER_DEF(std::u32string, 64, u32string)
GLOBAL_XXHASHER_DEF(std::u16string, 64, u16string)
GLOBAL_XXHASHER_DEF(std::u8string, 64, u8string)
GLOBAL_XXHASHER_DEF(std::string, 64, string)
GLOBAL_XXHASHER_DEF(std::wstring, 64, wstring)
GLOBAL_XXHASHER_DEF(stdfs::path, 64, path)

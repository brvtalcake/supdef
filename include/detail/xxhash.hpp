#ifndef XXHASH_HPP
#define XXHASH_HPP

#include <cstddef>
#include <cstdint>
#include <ctime>
#include <concepts>
#include <type_traits>
#include <optional>
#include <bit>

#if 0
#define XXH_INLINE_ALL 1
#else
#define XXH_NAMESPACE supdef_xxhash_impl_
#endif
#include <xxhash.h>

namespace supdef
{
    namespace detail
    {
        static_assert(std::same_as<uint64_t, size_t>);

        static inline __uint128_t hash128_bytes(const void* data, size_t size, std::optional<__uint128_t> seed = std::nullopt)
        {
            XXH128_hash_t tmp;
            if (!bool(seed))
                tmp = XXH3_128bits(data, size);
            else
                tmp = XXH3_128bits_withSeed(data, size, seed.value());
            /* XXH128_canonicalFromHash(tmp); */
            return std::bit_cast<__uint128_t>(tmp);
        }

        static inline uint64_t hash64_bytes(const void* data, size_t size, std::optional<uint64_t> seed = std::nullopt)
        {
            if (!bool(seed))
                return XXH3_64bits(data, size);
            else
                return XXH3_64bits_withSeed(data, size, seed.value());
        }

        static inline uint32_t hash32_bytes(const void* data, size_t size, std::optional<uint32_t> seed = std::nullopt)
        {
            uint64_t hashed = hash64_bytes(data, size, seed);
            // return xored upper and lower 32 bits
            uint32_t parts[2];
            ::memcpy(parts, &hashed, sizeof(parts));
            return parts[0] ^ parts[1];
        }

        template <typename HashedType, size_t HashSize>
        class xxhash;

        template <typename HashedType>
        class xxhash<HashedType, 128>
        {
        protected:
            static __uint128_t hash(const HashedType& data, std::optional<__uint128_t> seed = std::nullopt)
            {
                return hash128_bytes(&data, sizeof(data), seed);
            }

            static __uint128_t hash(const void* data, size_t size, std::optional<__uint128_t> seed = std::nullopt)
            {
                return hash128_bytes(data, size, seed);
            }

        public:
            xxhash() = delete;
            xxhash(std::optional<__uint128_t> seed = std::nullopt)
                : m_seed(seed)
            {
            }

            __uint128_t operator()(const HashedType& data, std::optional<__uint128_t> opt_seed = std::nullopt) const
            {
                return hash(data, opt_seed ?: m_seed);
            }

            __uint128_t operator()(const void* data, size_t size, std::optional<__uint128_t> opt_seed = std::nullopt) const
            {
                return hash(data, size, opt_seed ?: m_seed);
            }

        private:
            std::optional<__uint128_t> m_seed;
        };

        template <typename HashedType>
        class xxhash<HashedType, 64>
        {
        protected:
            static uint64_t hash(const HashedType& data, std::optional<uint64_t> seed = std::nullopt)
            {
                return hash64_bytes(&data, sizeof(data), seed);
            }

            static uint64_t hash(const void* data, size_t size, std::optional<uint64_t> seed = std::nullopt)
            {
                return hash64_bytes(data, size, seed);
            }

        public:
            xxhash() = delete;
            xxhash(std::optional<uint64_t> seed = std::nullopt)
                : m_seed(seed)
            {
            }

            uint64_t operator()(const HashedType& data, std::optional<uint64_t> opt_seed = std::nullopt) const
            {
                return hash(data, opt_seed ?: m_seed);
            }

            uint64_t operator()(const void* data, size_t size, std::optional<uint64_t> opt_seed = std::nullopt) const
            {
                return hash(data, size, opt_seed ?: m_seed);
            }

        private:
            std::optional<uint64_t> m_seed;
        };

        template <typename HashedType>
        class xxhash<HashedType, 32>
        {
        protected:
            static uint32_t hash(const HashedType& data, std::optional<uint32_t> seed = std::nullopt)
            {
                return hash32_bytes(&data, sizeof(data), seed);
            }

            static uint32_t hash(const void* data, size_t size, std::optional<uint32_t> seed = std::nullopt)
            {
                return hash32_bytes(data, size, seed);
            }

        public:
            xxhash() = delete;
            xxhash(std::optional<uint32_t> seed = std::nullopt)
                : m_seed(seed)
            {
            }

            uint32_t operator()(const HashedType& data, std::optional<uint32_t> opt_seed = std::nullopt) const
            {
                return hash(data, opt_seed ?: m_seed);
            }

            uint32_t operator()(const void* data, size_t size, std::optional<uint32_t> opt_seed = std::nullopt) const
            {
                return hash(data, size, opt_seed ?: m_seed);
            }

        private:
            std::optional<uint32_t> m_seed;
        };
    }
}

#endif

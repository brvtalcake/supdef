#ifndef ARENA_HPP
#define ARENA_HPP

#include <types.hpp>
#include <version.hpp>

#include <memory_resource>

namespace supdef
{
    // TODO: finish and test this
    template <size_t N>
    class arena
        : public std::pmr::memory_resource
    {
        struct mem_hdr
        {
            std::byte* prev, next; // previous and next block
            size_t before,   // free in-block space before header
                   after;    // free (or taken by user data) in-block space after header

            /*
             * Free space before the header (but inside the same block)
             * may be needed if the user requested alignment is so big that
             * there needs to be some padding between the header and user data.
             * Since we still want the header to be JUST BEFORE user data,
             * we need to potentially let free space before it.
             */
        };

        static constexpr void toggle_allocated(size_t& size) noexcept
        {
            size ^= ((size_t)1 << (sizeof(size_t) * CHAR_BIT - 1));
        }
        static constexpr void toggle_allocated(mem_hdr& hdr) noexcept
        {
            toggle_allocated(hdr.before);
        }

        static constexpr void mark_allocated(size_t& size) noexcept
        {
            size |= ((size_t)1 << (sizeof(size_t) * CHAR_BIT - 1));
        }
        static constexpr void mark_allocated(mem_hdr& hdr) noexcept
        {
            mark_allocated(hdr.before);
        }

        static constexpr void mark_unallocated(size_t& size) noexcept
        {
            size &= ~((size_t)1 << (sizeof(size_t) * CHAR_BIT - 1));
        }
        static constexpr void mark_unallocated(mem_hdr& hdr) noexcept
        {
            mark_unallocated(hdr.before);
        }

        static constexpr bool is_allocated(size_t size) noexcept
        {
            return size & ((size_t)1 << (sizeof(size_t) * CHAR_BIT - 1));
        }
        static constexpr bool is_allocated(mem_hdr hdr) noexcept
        {
            return is_allocated(hdr.before);
        }

        static constexpr size_t size(size_t before, size_t after) noexcept
        {
            size_t _before = before;
            mark_unallocated(_before);
            return _before + after;
        }
        static constexpr size_t size(mem_hdr hdr) noexcept
        {
            return size(hdr.before, hdr.after);
        }

        static constexpr bool is_power_of_two(size_t n) noexcept
        {
            return std::has_single_bit(n);
        }

        static constexpr uint8_t log2(size_t n) noexcept
        {
            return std::countr_zero(n);
        }

        static constexpr uintptr_t align_up(uintptr_t ptr, size_t align) noexcept
        {
            return (ptr + (align - 1)) & ~(align - 1);
        }

        static constexpr uintptr_t to_header(uintptr_t start) noexcept
        {
            return start - sizeof(mem_hdr);
        }

        static constexpr uintptr_t to_data(uintptr_t start) noexcept
        {
            return start + sizeof(mem_hdr);
        }

        static mem_hdr header_at(std::byte* ptr) const noexcept
        {
            mem_hdr hdr;
            ::memcpy(&hdr, ptr, sizeof(mem_hdr));
            return hdr;
        }

        static void header_at(std::byte* ptr, const mem_hdr& hdr) noexcept
        {
            ::memcpy(ptr, &hdr, sizeof(mem_hdr));
        }

        static constexpr std::optional<std::tuple<std::byte*, std::byte*, std::byte*, mem_hdr, std::optional<mem_hdr>>>
        can_contain(std::byte* addr, size_t bytes, size_t alignment) noexcept
        {
            const mem_hdr h = header_at(addr);
            if (is_allocated(h))
                return std::nullopt;

            const uintptr_t block_start = reinterpret_cast<uintptr_t>(addr) - h.before;
            const uintptr_t block_end = block_start + h.before + sizeof(mem_hdr) + h.after;
            const uintptr_t block_size = block_end - block_start;

            const uintptr_t potential_start = block_start + sizeof(mem_hdr);
            const uintptr_t suitably_aligned_start = align_up(potential_start, alignment);
            const size_t padding = suitably_aligned_start - potential_start;
            const size_t full_needed_size = padding         // padding until user data (which will actually end up BEFORE the header)
                                          + sizeof(mem_hdr) // header
                                          + bytes;          // user data

            if (full_needed_size <= block_size)
            {
                mem_hdr updated_hdr;
                
                if (full_needed_size + sizeof(mem_hdr) < block_size)
                {
                    std::byte* const updated_block_end = reinterpret_cast<std::byte*>(block_start) + full_needed_size;

                    // split
                    updated_hdr.prev = h.prev;
                    updated_hdr.next = updated_block_end;
                    updated_hdr.before = padding;
                    updated_hdr.after = bytes;
                    mark_allocated(updated_hdr);

                    mem_hdr new_next_hdr;
                    new_next_hdr.prev = reinterpret_cast<std::byte*>(block_start) + padding;
                    new_next_hdr.next = h.next;
                    new_next_hdr.before = 0;
                    new_next_hdr.after = block_end - reinterpret_cast<uintptr_t>(updated_block_end) - sizeof(mem_hdr);
                    mark_unallocated(new_next_hdr);

                    return std::make_tuple(
                        reinterpret_cast<std::byte*>(block_start) + padding, // start of header
                        reinterpret_cast<std::byte*>(block_start) + padding + sizeof(mem_hdr), // start of user data
                        updated_block_end, // end of block (and location for the 'new_next_hdr')
                        updated_hdr, new_next_hdr
                    );
                }
                else
                {
                    // no split
                    updated_hdr.prev = h.prev;
                    updated_hdr.next = h.next;
                    updated_hdr.before = padding;
                    updated_hdr.after = block_end - suitably_aligned_start;
                    mark_allocated(updated_hdr);

                    return std::make_tuple(
                        reinterpret_cast<std::byte*>(block_start) + padding, // start of header
                        reinterpret_cast<std::byte*>(block_start) + padding + sizeof(mem_hdr), // start of user data
                        nullptr,              // end of block (no new block, so normally not used)
                        updated_hdr, std::nullopt
                    );
                }
            }
            return std::nullopt;
        }

    public:
        arena()
            : m_data()
            , m_search_from(reinterpret_cast<uintptr_t>(m_data))
        {
            if (N < sizeof(mem_hdr))
                throw std::invalid_argument("arena size is too small");
            if (N & ((size_t)1 << (sizeof(size_t) * CHAR_BIT - 1)))
                throw std::invalid_argument("arena size is too large");

            mem_hdr hdr;
            hdr.next = m_data;
            hdr.before = 0;
            hdr.after = N - sizeof(mem_hdr);
            mark_unallocated(hdr);
            header_at(m_data, hdr);
        }

        ~arena() override = default;

    protected:
        virtual void* do_allocate(size_t bytes, size_t alignment = __STDCPP_DEFAULT_NEW_ALIGNMENT__) override
        {
            if (!is_power_of_two(alignment))
                throw std::invalid_argument("alignment is not a power of two");

#if SUPDEF_MULTITHREADED
            std::lock_guard lock(m_mutex);
#endif
            std::byte* addr = reinterpret_cast<std::byte*>(m_search_from);
            while (addr)
            {
                auto can_contain_res = can_contain(addr, bytes, alignment);
                if (can_contain_res.has_value())
                {
                    auto [hdr_start, data_start, next_block, hdr, next_hdr] = can_contain_res.value();
                    
                    header_at(hdr_start, hdr);

                    if (next_hdr.has_value())
                    {
                        header_at(next_block, next_hdr.value());
                        m_search_from = reinterpret_cast<uintptr_t>(next_block);
                    }
                    else
                        m_search_from = reinterpret_cast<uintptr_t>(hdr.next);

                    
                    mem_hdr prev = header_at(hdr.prev);
                    prev.next = hdr_start;
                    header_at(hdr.prev, prev);

                    return data_start;
                }
                next_header(addr);
            }

            return nullptr;
        }
    private:
        alignas(std::max_align_t) std::byte m_data[N];
        uintptr_t m_search_from;
#if SUPDEF_MULTITHREADED
        std::recursive_mutex m_mutex;
#endif
    };
}

#endif

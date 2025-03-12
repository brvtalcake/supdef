#ifndef ARENA_HPP
#define ARENA_HPP

#include <types.hpp>
#include <version.hpp>

#include <memory_resource>

namespace supdef
{
    namespace detail
    {
        PACKED_STRUCT(mem_hdr)
        {
            uintptr_t xord_prev_next; // previous and next headers
            size_t before : 16;       // quantity to substract from the header's address to get the start of the managed block
            size_t after  : 47;       // quantity to add to the header's address to get the end of the managed block
            bool is_allocated : 1;    // whether the block is allocated or not

            static constexpr size_t max_alignment = (1 << 17) - 1;
            static constexpr size_t max_size = (1 << 47) - 1;

            constexpr std::byte* prev(std::byte* next) const noexcept
            {
                return reinterpret_cast<std::byte*>(this->xord_prev_next ^ reinterpret_cast<uintptr_t>(next));
            }
            constexpr std::byte* next(std::byte* prev) const noexcept
            {
                return reinterpret_cast<std::byte*>(this->xord_prev_next ^ reinterpret_cast<uintptr_t>(prev));
            }

            constexpr void set_neighbours(std::byte* prev, std::byte* next) noexcept
            {
                this->xord_prev_next = reinterpret_cast<uintptr_t>(prev) ^ reinterpret_cast<uintptr_t>(next);
            }

            constexpr size_t size() const noexcept
            {
                return this->before + this->after;
            }
            constexpr size_t user_size() const noexcept
            {
                return this->after;
            }

            constexpr void mark_allocated() noexcept
            {
                this->is_allocated = true;
            }
            constexpr void mark_unallocated() noexcept
            {
                this->is_allocated = false;
            }
            constexpr void toggle_allocated() noexcept
            {
                this->is_allocated = !this->is_allocated;
            }

            constexpr std::tuple<std::byte*, std::byte*> block(std::byte* self_addr) const noexcept
            {
                return std::make_tuple(
                    self_addr - this->before,
                    self_addr + sizeof(mem_hdr) + this->after
                );
            }
            constexpr void block(std::byte* self_addr, std::byte* block_start, std::byte* block_end) noexcept
            {
                this->before = self_addr - block_start;
                this->after = block_end - self_addr - sizeof(mem_hdr);
            }

            constexpr std::tuple<std::byte*, std::byte*> user(std::byte* self_addr) const noexcept
            {
                return std::make_tuple(
                    self_addr + sizeof(mem_hdr),
                    self_addr + sizeof(mem_hdr) + this->after
                );
            }

            static constexpr mem_hdr at(std::byte* ptr) noexcept;
            static constexpr void at(std::byte* ptr, const mem_hdr& hdr) noexcept;

            static constexpr mem_hdr at(uintptr_t ptr) noexcept;
            static constexpr void at(uintptr_t ptr, const mem_hdr& hdr) noexcept;

            /*
            * Free space before the header (but inside the same block)
            * may be needed if the user requested alignment is so big that
            * there needs to be some padding between the header and user data.
            * Since we still want the header to be JUST BEFORE user data,
            * we need to potentially let free space before it.
            */
        };
        static_assert(std::is_trivially_copyable_v<mem_hdr>);
        static_assert(std::is_standard_layout_v<mem_hdr>);
        static_assert(sizeof(mem_hdr) == 2 * 8);

        class arena_base
        {
        protected:
            struct block_info
            {
                mem_hdr hdr;
                std::byte* hdr_addr;
            };

            static inline constexpr bool is_power_of_two(size_t n) noexcept
            {
                return std::has_single_bit(n);
            }

            static inline constexpr uint8_t log2(size_t n) noexcept
            {
                return std::countr_zero(n);
            }

            static inline constexpr uintptr_t align_up(uintptr_t ptr, size_t align) noexcept
            {
                return (ptr + (align - 1)) & ~(align - 1);
            }
            static inline constexpr std::byte* align_up(std::byte* ptr, size_t align) noexcept
            {
                return reinterpret_cast<std::byte*>(align_up(reinterpret_cast<uintptr_t>(ptr), align));
            }

            static inline constexpr uintptr_t to_header(uintptr_t start) noexcept
            {
                return start - sizeof(mem_hdr);
            }

            static inline constexpr uintptr_t to_data(uintptr_t start) noexcept
            {
                return start + sizeof(mem_hdr);
            }

            static inline constexpr std::optional<std::tuple<std::byte*, std::byte*>>
            find_suitable(const block_info& h, size_t bytes, size_t alignment) noexcept
            {
                auto [block_start, block_end] = h.hdr.block(h.hdr_addr);
                auto user_start = align_up(block_start + sizeof(mem_hdr), alignment);
                if (user_start + bytes <= block_end)
                    return std::make_tuple(user_start, user_start + bytes);
                return std::nullopt;
            }
        };

        template <size_t N>
            requires (N > 2 * sizeof(mem_hdr)) && (N <= mem_hdr::max_size)
        class arena_impl
            : public arena_base
        {
            static constexpr std::optional<std::tuple<std::byte*, std::byte*>>
            can_contain(const block_info& h, size_t bytes, size_t alignment) noexcept
            {
                if (h.hdr.is_allocated)
                    return std::nullopt;

                return find_suitable(h, bytes, alignment);
            }

            static constexpr inline bool can_split(std::byte* user_end, std::byte* block_end) noexcept
            {
                return user_end + sizeof(mem_hdr) < block_end;
            }

            // allocated the block, and then if possible splits it in two
            void* do_alloc(std::byte* prev_addr, const block_info& b, std::tuple<std::byte*, std::byte*> user) noexcept
            {
                const auto old_blk_bounds = b.hdr.block(b.hdr_addr);
                const bool needs_split = can_split(std::get<1>(user), std::get<1>(old_blk_bounds));

                block_info new_block;
                new_block.hdr_addr = std::get<0>(user) - sizeof(mem_hdr);
                new_block.hdr = b.hdr;
                new_block.hdr.block(
                    new_block.hdr_addr,
                    std::get<0>(old_blk_bounds),
                    needs_split ? std::get<1>(user)
                                : std::get<1>(old_blk_bounds)
                );
                new_block.hdr.mark_allocated();

                if (needs_split)
                {
                    block_info next_block;
                    next_block.hdr_addr = std::get<1>(user);
                    new_block.hdr.set_neighbours(prev_addr, std::get<1>(user));

                    next_block.hdr.set_neighbours(new_block.hdr_addr, b.hdr.next(b.hdr_addr));
                    next_block.hdr.block(
                        next_block.hdr_addr,
                        std::get<1>(user),
                        std::get<1>(old_blk_bounds)
                    );
                    next_block.hdr.mark_unallocated();

                    mem_hdr old_next_hdr = mem_hdr::at(b.hdr.next(b.hdr_addr));
                    std::byte* old_next_hdr_old_next = old_next_hdr.next(b.hdr_addr);
                    old_next_hdr.set_neighbours(new_block.hdr_addr, old_next_hdr_old_next);
                    mem_hdr::at(b.hdr.next(b.hdr_addr), old_next_hdr);

                    mem_hdr::at(new_block.hdr_addr, new_block.hdr);
                    mem_hdr::at(next_block.hdr_addr, next_block.hdr);

                    return std::get<0>(user);
                }
                else
                {
                    mem_hdr::at(new_block.hdr_addr, new_block.hdr);

                    return std::get<0>(user);
                }
            }

        protected:
            arena_impl()
                : m_data()
                , m_search_from(reinterpret_cast<uintptr_t>(m_data + sizeof(mem_hdr)))
                , m_prev_search_from(reinterpret_cast<uintptr_t>(m_data))
            {
                std::byte* prev;
                std::byte* next;

                mem_hdr first;
                mem_hdr middle;
                mem_hdr last;

                prev = nullptr;
                next = m_data + sizeof(mem_hdr);
                first.set_neighbours(prev, next);
                first.block(m_data, m_data, m_data + N);
                first.mark_allocated();

                prev = m_data;
                next = m_data + N - sizeof(mem_hdr);
                middle.set_neighbours(prev, next);
                middle.block(m_data + sizeof(mem_hdr), m_data + sizeof(mem_hdr), m_data + N - sizeof(mem_hdr));
                middle.mark_unallocated();

                prev = m_data + N - 2 * sizeof(mem_hdr);
                next = nullptr;
                last.set_neighbours(prev, next);
                last.block(m_data + N - sizeof(mem_hdr), m_data + N - sizeof(mem_hdr), m_data + N);
                last.mark_allocated();

                mem_hdr::at(m_data, hdr);
                mem_hdr::at(m_data + sizeof(mem_hdr), middle);
                mem_hdr::at(m_data + N - sizeof(mem_hdr), last);
            }

            ~arena_impl() = default;

            void* alloc_aligned(size_t bytes, size_t alignment = __STDCPP_DEFAULT_NEW_ALIGNMENT__)
            {
                if (!is_power_of_two(alignment))
                    throw std::invalid_argument("alignment is not a power of two");

#if SUPDEF_MULTITHREADED
                std::lock_guard lock(m_mutex);
#endif
                std::byte* addr = reinterpret_cast<std::byte*>(m_search_from);
                std::byte* prev_addr = reinterpret_cast<std::byte*>(m_prev_search_from);
                while (addr)
                {
                    block_info b;
                    b.hdr = mem_hdr::at(addr);
                    b.hdr_addr = addr;
                    auto can_contain_res = can_contain(b, bytes, alignment);
                    if (can_contain_res)
                        return this->do_alloc(prev_addr, b, *can_contain_res);
                    next_header(prev_addr, addr);
                }

                return nullptr;
            }
        private:
            ATTRIBUTE_UNINITIALIZED
                alignas(std::max_align_t)
                    std::byte m_data[N];
            uintptr_t m_search_from;
            uintptr_t m_prev_search_from;
#if SUPDEF_MULTITHREADED
            std::recursive_mutex m_mutex;
#endif
        };
    }
    // TODO: finish and test this
    template <size_t N>
    class arena
        : public std::pmr::memory_resource
    {
        
    };
}

constexpr supdef::detail::mem_hdr
supdef::detail::mem_hdr::at(std::byte* ptr) noexcept
{
    mem_hdr hdr;
    ::memcpy(&hdr, ptr, sizeof(mem_hdr));
    return hdr;
}

constexpr void
supdef::detail::mem_hdr::at(std::byte* ptr, const supdef::detail::mem_hdr& hdr) noexcept
{
    ::memcpy(ptr, &hdr, sizeof(mem_hdr));
}

constexpr supdef::detail::mem_hdr
supdef::detail::mem_hdr::at(uintptr_t ptr) noexcept
{
    return at(reinterpret_cast<std::byte*>(ptr));
}

constexpr void
supdef::detail::mem_hdr::at(uintptr_t ptr, const supdef::detail::mem_hdr& hdr) noexcept
{
    at(reinterpret_cast<std::byte*>(ptr), hdr);
}

#endif

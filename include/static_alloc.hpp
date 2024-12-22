#ifndef STATIC_ALLOC_HPP
#define STATIC_ALLOC_HPP

#include <memory>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <array>

namespace supdef
{
    template <typename T, size_t N = 1024>
    class static_allocator
    {
    public:
        using value_type = T;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using const_pointer = const T*;
        using reference = T&;
        using const_reference = const T&;

        template <typename U>
        struct rebind
        {
            using other = static_allocator<U, N>;
        };

        static_allocator() noexcept
            : m_data(), m_size(0)
        { }

        static_allocator(const static_allocator& other) noexcept
            : m_data(), m_size(other.m_size)
        {
            std::copy(other.m_data, other.m_data + m_size, m_data);
        }

        template <typename U>
        static_allocator(const static_allocator<U, N>& other) noexcept
            : m_data(), m_size(other.m_size)
        {
            std::copy(other.m_data, other.m_data + m_size, m_data);
        }

        ~static_allocator() = default;

        pointer allocate(size_type n)
        {
            if (n > N)
                throw std::bad_alloc();
            return m_data;
        }

        void deallocate(pointer p, size_type n) noexcept
        {
            if (p != m_data)
                throw std::invalid_argument("invalid pointer");
            if (n > N)
                throw std::invalid_argument("invalid size");
        }

        size_type max_size() const noexcept
        {
            return N;
        }

        template <typename U, typename... Args>
        void construct(U* p, Args&&... args)
        {
            new (p) U(std::forward<Args>(args)...);
        }

        template <typename U>
        void destroy(U* p)
        {
            p->~U();
        }

        pointer address(reference x) const noexcept
        {
            return &x;
        }

        const_pointer address(const_reference x) const noexcept
        {
            return &x;
        }

        bool operator==(const static_allocator& other) const noexcept
        {
            return m_data == other.m_data;
        }

        bool operator!=(const static_allocator& other) const noexcept
        {
            return !operator==(other);
        }

    private:
        T m_data[N];
        size_type m_size;
    };
}

#endif

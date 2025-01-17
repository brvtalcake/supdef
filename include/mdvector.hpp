#ifndef MDVECTOR_HPP
#define MDVECTOR_HPP

#include <vector>
#include <memory>
#include <algorithm>
#include <iterator>
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <stdexcept>
#include <concepts>
#include <functional>

namespace supdef
{
#if 0
    template <typename T, typename Allocator = std::allocator<T>>
    class mdvector
    {
        void delete_func(T* ptr) const noexcept
        {
            m_alloc.deallocate(ptr, m_capacity);
        }
        T* alloc_func(size_t size) const noexcept
        {
            return m_alloc.allocate(size);
        }
        T* realloc_func(T* ptr, size_t size) const noexcept
        {
            return m_alloc.reallocate(ptr, size);
        }
    public:
        struct mdvector_iterator
        {
            // TODO
        };
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using iterator = mdvector_iterator;
        using const_iterator = const mdvector_iterator;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        constexpr mdvector();
    private:
        using deleter_type = std::move_only_function<void(mdvector::*)(T*) const noexcept>;
        constexpr static inline deleter_type m_delete_func = &mdvector::delete_func;
        Allocator m_alloc;
        std::unique_ptr<T[], deleter_type> m_data;
        std::unique_ptr<size_t[], deleter_type> m_sizes;
        size_type m_full_size;
        size_type m_capacity;
        size_t m_dims;
    };
#endif
}

#endif

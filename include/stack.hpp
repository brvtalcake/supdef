#ifndef STACK_HPP
#define STACK_HPP

#include <types.hpp>

#include <deque>
#include <stack>

namespace supdef
{
    template <typename T, typename ContainerT = typename std::stack<T>::container_type>
    class stack
        : public std::stack<T, ContainerT>
    {
    private:
        using base_type = std::stack<T, ContainerT>;
        using underlying_type = typename base_type::container_type;

        template <typename Self>
        using ref_type = std::conditional_t<
            std::is_const_v<Self>,
            typename underlying_type::const_reference,
            typename underlying_type::reference
        >;
    
    protected:
        using base_type::c;
    
    public:
        using base_type::base_type;

        constexpr void make_empty()
        {
            while (!this->empty())
                this->pop();
        }

        template <typename Self, typename RetT, typename... ArgsBefore, typename... ArgsAfter>
        constexpr void traverse(
            this Self&& self,
            std::function<RetT(ArgsBefore..., ref_type<Self&&>, ArgsAfter...)>& func,
            ArgsBefore&&... args_before,
            ArgsAfter&&... args_after
        )
        {
            if constexpr (std::is_const_v<Self&&>)
            {
                for (auto it = self.c.crbegin(); it != self.c.crend(); ++it)
                    std::invoke(func, args_before..., *it, args_after...);
            }
            else
            {
                for (auto it = self.c.rbegin(); it != self.c.rend(); ++it)
                    std::invoke(func, args_before..., *it, args_after...);
            }
        }

        template <typename Self, typename RetT, typename... ArgsBefore>
        constexpr void traverse(
            this Self&& self,
            std::function<RetT(ArgsBefore..., ref_type<Self&&>)>& func,
            ArgsBefore&&... args_before
        )
        {
            if constexpr (std::is_const_v<Self&&>)
            {
                for (auto it = self.c.crbegin(); it != self.c.crend(); ++it)
                    std::invoke(func, args_before..., *it);
            }
            else
            {
                for (auto it = self.c.rbegin(); it != self.c.rend(); ++it)
                    std::invoke(func, args_before..., *it);
            }
        }

        template <typename Self, typename RetT, typename... ArgsAfter>
        constexpr void traverse(
            this Self&& self,
            std::function<RetT(ref_type<Self&&>, ArgsAfter...)>& func,
            ArgsAfter&&... args_after
        )
        {
            if constexpr (std::is_const_v<Self&&>)
            {
                for (auto it = self.c.crbegin(); it != self.c.crend(); ++it)
                    std::invoke(func, *it, args_after...);
            }
            else
            {
                for (auto it = self.c.rbegin(); it != self.c.rend(); ++it)
                    std::invoke(func, *it, args_after...);
            }
        }

        template <typename Self, typename RetT>
        constexpr void traverse(
            this Self&& self,
            std::function<RetT(ref_type<Self&&>)>& func
        )
        {
            if constexpr (std::is_const_v<Self&&>)
            {
                for (auto it = self.c.crbegin(); it != self.c.crend(); ++it)
                    std::invoke(func, *it);
            }
            else
            {
                for (auto it = self.c.rbegin(); it != self.c.rend(); ++it)
                    std::invoke(func, *it);
            }
        }

        // same, but with noexcept
        template <typename Self, typename RetT, typename... ArgsBefore, typename... ArgsAfter>
        constexpr void traverse(
            this Self&& self,
            std::function<RetT(ArgsBefore..., ref_type<Self&&>, ArgsAfter...) noexcept>& func,
            ArgsBefore&&... args_before,
            ArgsAfter&&... args_after
        ) noexcept
        {
            if constexpr (std::is_const_v<Self&&>)
            {
                for (auto it = self.c.crbegin(); it != self.c.crend(); ++it)
                    std::invoke(func, args_before..., *it, args_after...);
            }
            else
            {
                for (auto it = self.c.rbegin(); it != self.c.rend(); ++it)
                    std::invoke(func, args_before..., *it, args_after...);
            }
        }

        template <typename Self, typename RetT, typename... ArgsBefore>
        constexpr void traverse(
            this Self&& self,
            std::function<RetT(ArgsBefore..., ref_type<Self&&>) noexcept>& func,
            ArgsBefore&&... args_before
        ) noexcept
        {
            if constexpr (std::is_const_v<Self&&>)
            {
                for (auto it = self.c.crbegin(); it != self.c.crend(); ++it)
                    std::invoke(func, args_before..., *it);
            }
            else
            {
                for (auto it = self.c.rbegin(); it != self.c.rend(); ++it)
                    std::invoke(func, args_before..., *it);
            }
        }

        template <typename Self, typename RetT, typename... ArgsAfter>
        constexpr void traverse(
            this Self&& self,
            std::function<RetT(ref_type<Self&&>, ArgsAfter...) noexcept>& func,
            ArgsAfter&&... args_after
        ) noexcept
        {
            if constexpr (std::is_const_v<Self&&>)
            {
                for (auto it = self.c.crbegin(); it != self.c.crend(); ++it)
                    std::invoke(func, *it, args_after...);
            }
            else
            {
                for (auto it = self.c.rbegin(); it != self.c.rend(); ++it)
                    std::invoke(func, *it, args_after...);
            }
        }

        template <typename Self, typename RetT>
        constexpr void traverse(
            this Self&& self,
            std::function<RetT(ref_type<Self&&>) noexcept>& func
        ) noexcept
        {
            if constexpr (std::is_const_v<Self&&>)
            {
                for (auto it = self.c.crbegin(); it != self.c.crend(); ++it)
                    std::invoke(func, *it);
            }
            else
            {
                for (auto it = self.c.rbegin(); it != self.c.rend(); ++it)
                    std::invoke(func, *it);
            }
        }

        template <typename Self, typename... ArgsBefore, typename... ArgsAfter>
        constexpr std::pair<size_t, bool> traverse_until(
            this Self&& self,
            std::function<bool(ArgsBefore..., ref_type<Self&&>, ArgsAfter...)>& func,
            ArgsBefore&&... args_before,
            ArgsAfter&&... args_after
        )
        {
            size_t count = 0;
            bool gotit = false;
            if constexpr (std::is_const_v<Self&&>)
            {
                for (auto it = self.c.crbegin(); it != self.c.crend(); ++it)
                {
                    if (std::invoke(func, args_before..., *it, args_after...))
                    {
                        gotit = true;
                        break;
                    }
                    count++;
                }
            }
            else
            {
                for (auto it = self.c.rbegin(); it != self.c.rend(); ++it)
                {
                    if (std::invoke(func, args_before..., *it, args_after...))
                    {
                        gotit = true;
                        break;
                    }
                    count++;
                }
            }
            return {count, gotit};
        }

        template <typename Self, typename... ArgsBefore>
        constexpr std::pair<size_t, bool> traverse_until(
            this Self&& self,
            std::function<bool(ArgsBefore..., ref_type<Self&&>)>& func,
            ArgsBefore&&... args_before
        )
        {
            size_t count = 0;
            bool gotit = false;
            if constexpr (std::is_const_v<Self&&>)
            {
                for (auto it = self.c.crbegin(); it != self.c.crend(); ++it)
                {
                    if (std::invoke(func, args_before..., *it))
                    {
                        gotit = true;
                        break;
                    }
                    count++;
                }
            }
            else
            {
                for (auto it = self.c.rbegin(); it != self.c.rend(); ++it)
                {
                    if (std::invoke(func, args_before..., *it))
                    {
                        gotit = true;
                        break;
                    }
                    count++;
                }
            }
            return {count, gotit};
        }

        template <typename Self, typename... ArgsAfter>
        constexpr std::pair<size_t, bool> traverse_until(
            this Self&& self,
            std::function<bool(ref_type<Self&&>, ArgsAfter...)>& func,
            ArgsAfter&&... args_after
        )
        {
            size_t count = 0;
            bool gotit = false;
            if constexpr (std::is_const_v<Self&&>)
            {
                for (auto it = self.c.crbegin(); it != self.c.crend(); ++it)
                {
                    if (std::invoke(func, *it, args_after...))
                    {
                        gotit = true;
                        break;
                    }
                    count++;
                }
            }
            else
            {
                for (auto it = self.c.rbegin(); it != self.c.rend(); ++it)
                {
                    if (std::invoke(func, *it, args_after...))
                    {
                        gotit = true;
                        break;
                    }
                    count++;
                }
            }
            return {count, gotit};
        }

        template <typename Self>
        constexpr std::pair<size_t, bool> traverse_until(
            this Self&& self,
            std::function<bool(ref_type<Self&&>)>& func
        )
        {
            size_t count = 0;
            bool gotit = false;
            if constexpr (std::is_const_v<Self&&>)
            {
                for (auto it = self.c.crbegin(); it != self.c.crend(); ++it)
                {
                    if (std::invoke(func, *it))
                    {
                        gotit = true;
                        break;
                    }
                    count++;
                }
            }
            else
            {
                for (auto it = self.c.rbegin(); it != self.c.rend(); ++it)
                {
                    if (std::invoke(func, *it))
                    {
                        gotit = true;
                        break;
                    }
                    count++;
                }
            }
            return {count, gotit};
        }

        // same, but with noexcept
        template <typename Self, typename... ArgsBefore, typename... ArgsAfter>
        constexpr std::pair<size_t, bool> traverse_until(
            this Self&& self,
            std::function<bool(ArgsBefore..., ref_type<Self&&>, ArgsAfter...) noexcept>& func,
            ArgsBefore&&... args_before,
            ArgsAfter&&... args_after
        ) noexcept
        {
            size_t count = 0;
            bool gotit = false;
            if constexpr (std::is_const_v<Self&&>)
            {
                for (auto it = self.c.crbegin(); it != self.c.crend(); ++it)
                {
                    if (std::invoke(func, args_before..., *it, args_after...))
                    {
                        gotit = true;
                        break;
                    }
                    count++;
                }
            }
            else
            {
                for (auto it = self.c.rbegin(); it != self.c.rend(); ++it)
                {
                    if (std::invoke(func, args_before..., *it, args_after...))
                    {
                        gotit = true;
                        break;
                    }
                    count++;
                }
            }
            return {count, gotit};
        }

        template <typename Self, typename... ArgsBefore>
        constexpr std::pair<size_t, bool> traverse_until(
            this Self&& self,
            std::function<bool(ArgsBefore..., ref_type<Self&&>) noexcept>& func,
            ArgsBefore&&... args_before
        ) noexcept
        {
            size_t count = 0;
            bool gotit = false;
            if constexpr (std::is_const_v<Self&&>)
            {
                for (auto it = self.c.crbegin(); it != self.c.crend(); ++it)
                {
                    if (std::invoke(func, args_before..., *it))
                    {
                        gotit = true;
                        break;
                    }
                    count++;
                }
            }
            else
            {
                for (auto it = self.c.rbegin(); it != self.c.rend(); ++it)
                {
                    if (std::invoke(func, args_before..., *it))
                    {
                        gotit = true;
                        break;
                    }
                    count++;
                }
            }
            return {count, gotit};
        }

        template <typename Self, typename... ArgsAfter>
        constexpr std::pair<size_t, bool> traverse_until(
            this Self&& self,
            std::function<bool(ref_type<Self&&>, ArgsAfter...) noexcept>& func,
            ArgsAfter&&... args_after
        ) noexcept
        {
            size_t count = 0;
            bool gotit = false;
            if constexpr (std::is_const_v<Self&&>)
            {
                for (auto it = self.c.crbegin(); it != self.c.crend(); ++it)
                {
                    if (std::invoke(func, *it, args_after...))
                    {
                        gotit = true;
                        break;
                    }
                    count++;
                }
            }
            else
            {
                for (auto it = self.c.rbegin(); it != self.c.rend(); ++it)
                {
                    if (std::invoke(func, *it, args_after...))
                    {
                        gotit = true;
                        break;
                    }
                    count++;
                }
            }
            return {count, gotit};
        }

        template <typename Self>
        constexpr std::pair<size_t, bool> traverse_until(
            this Self&& self,
            std::function<bool(ref_type<Self&&>) noexcept>& func
        ) noexcept
        {
            size_t count = 0;
            bool gotit = false;
            if constexpr (std::is_const_v<Self&&>)
            {
                for (auto it = self.c.crbegin(); it != self.c.crend(); ++it)
                {
                    if (std::invoke(func, *it))
                    {
                        gotit = true;
                        break;
                    }
                    count++;
                }
            }
            else
            {
                for (auto it = self.c.rbegin(); it != self.c.rend(); ++it)
                {
                    if (std::invoke(func, *it))
                    {
                        gotit = true;
                        break;
                    }
                    count++;
                }
            }
            return {count, gotit};
        }

        template <typename Self>
        ref_type<Self&&> operator[](
            this Self&& self,
            size_t index
        )
        {
            ref_type<Self&&> it;
            if constexpr (std::is_const_v<Self&&>)
                it = self.c.crbegin();
            else
                it = self.c.rbegin();
            return *std::next(it, index);
        }
    };
}

#endif

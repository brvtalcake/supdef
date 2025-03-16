#ifndef CONSTEXPR_MAP_HPP
#define CONSTEXPR_MAP_HPP

namespace supdef
{
#if 0
    namespace detail
    {
        template <typename... T>
            requires (sizeof...(T) > 0) && std::__pair_like<T...>
        struct constexpr_map_storage
        {
            std::tuple<T...> data;

            constexpr constexpr_map_storage() = default;

            constexpr constexpr_map_storage(T&&... t)
                : data(std::forward<T>(t)...)
            {
            }
        };

        template <typename K, typename T>
        concept key_type_matches = std::same_as<std::remove_cvref_t<K>, std::remove_cvref_t<T>>;

        template <typename K, typename... T>
        struct constexpr_map_at_key
        {
            template <typename U>
                requires std::__pair_like<U>
            static constexpr decltype(auto) get_first(U&& u)
            {
                return std::get<0>(std::forward<U>(u));
            }

            template <typename U>
                requires std::__pair_like<U>
            static constexpr decltype(auto) get_second(U&& u)
            {
                return std::get<1>(std::forward<U>(u));
            }

            template <size_t I = 0>
            static constexpr decltype(auto) get(const std::tuple<T...>& data, K&& k)
            {
                if constexpr (I == sizeof...(T))
                    return nothing;
                else if constexpr (key_type_matches<K, std::tuple_element_t<I, T>> && get_first(std::get<I>(data)) == k)
                    return get_second(std::get<I>(data));
                else
                    return get<I + 1>(data, std::forward<K>(k));
            }
        };

        template <typename... Fn, typename... T>
            requires (sizeof...(Fn) == sizeof...(T)) && std::__pair_like<T...>
        struct constexpr_map_for_each
        {
            template <typename Fn, size_t... I>
            static constexpr auto create_call_table_impl(Fn&& fn, std::index_sequence<I...>)
            {
                return std::tuple
            }

            template <typename Fn>
            static constexpr auto create_call_table(Fn&& fn)
            {
                return create_call_table_impl(std::forward<Fn>(fn), std::index_sequence_for<T...>{});
            }

            template <size_t I = 0>
            static constexpr void apply(const std::tuple<T...>& data, Fn&&... fn)
            {
                if constexpr (I < sizeof...(Fn))
                {
                    std::invoke(std::forward<Fn>(fn), std::get<I>(data));
                    apply<I + 1>(data, std::forward<Fn>(fn)...);
                }
            }
        };
    }

    template <typename... T>
    struct constexpr_map : detail::constexpr_map_storage<T...>
    {
        using detail::constexpr_map_storage<T...>::data;

        constexpr constexpr_map() = default;

        constexpr constexpr_map(T&&... t)
            : detail::constexpr_map_storage<T...>(std::forward<T>(t)...)
        {
        }

        template <typename K>
        constexpr decltype(auto) operator[](K&& k) const
        {
            return detail::constexpr_map_at_key<K, T...>::get(this->data, std::forward<K>(k));
        }

        template <typename Fn>
        constexpr void for_each(Fn&& fn) const
        {
            using foreach = detail::constexpr_map_for_each<Fn, T...>;
            constexpr auto call_table = foreach::create_call_table(std::forward<Fn>(fn));
            /* foreach::apply(this->data,  */
        }
    };
#endif
}

#endif

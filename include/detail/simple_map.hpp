#ifndef SIMPLE_MAP_HPP
#define SIMPLE_MAP_HPP

#include <types.hpp>

namespace supdef
{
#if 0
    template <typename... T1, typename... T2, size_t... Is>
        requires (sizeof...(T1) == sizeof...(T2)) && (sizeof...(T1) == sizeof...(Is))
    static constexpr decltype(auto) zip_tuples_impl(
        const std::tuple<T1...>& t1, const std::tuple<T2...>& t2, std::index_sequence<Is...>
    ) {
        return std::make_tuple(
            std::make_pair(std::get<Is>(t1), std::get<Is>(t2))...
        );
    }

    template <typename... T1, typename... T2, size_t... Is>
        requires (sizeof...(T1) == sizeof...(T2)) && (sizeof...(T1) == sizeof...(Is))
    static constexpr decltype(auto) zip_tuples_impl(
        std::tuple<T1...>&& t1, std::tuple<T2...>&& t2, std::index_sequence<Is...>
    ) {
        return std::make_tuple(
            std::make_pair(
                std::get<Is>(std::move(t1)),
                std::get<Is>(std::move(t2))
            )...
        );
    }

    template <typename... T1, typename... T2, size_t... Is>
        requires (sizeof...(T1) == sizeof...(T2)) && (sizeof...(T1) == sizeof...(Is))
    static constexpr decltype(auto) zip_tuples_impl(
        const std::tuple<T1...>& t1, std::tuple<T2...>&& t2, std::index_sequence<Is...>
    ) {
        return std::make_tuple(
            std::make_pair(
                std::get<Is>(t1),
                std::get<Is>(std::move(t2))
            )...
        );
    }

    template <typename... T1, typename... T2, size_t... Is>
        requires (sizeof...(T1) == sizeof...(T2)) && (sizeof...(T1) == sizeof...(Is))
    static constexpr decltype(auto) zip_tuples_impl(
        std::tuple<T1...>&& t1, const std::tuple<T2...>& t2, std::index_sequence<Is...>
    ) {
        return std::make_tuple(
            std::make_pair(
                std::get<Is>(std::move(t1)),
                std::get<Is>(t2)
            )...
        );
    }
    
    template <typename TP1, typename TP2>
    static constexpr decltype(auto) zip_tuples(TP1&& t1, TP2&& t2)
    {
        return zip_tuples_impl(
            FWD_AUTO(t1), FWD_AUTO(t2), index_sequence_for_tuple<std::remove_cvref_t<TP1>>{}
        );
    }

    /**
     * @brief Simple, constexpr map implementation.
     * @tparam Keys
     * @tparam Values
     */
    template <typename... Keys, typename... Values>
        requires (sizeof...(Keys) == sizeof...(Values)) && (sizeof...(Keys) > 0)
    class simple_map
    {
        template <typename KeyT, typename ValueT>
        class item_storage
        {
        public:
            using key_type = KeyT;
            using value_type = ValueT;

            constexpr item_storage() = default;

            template <typename KT, typename VT>
                requires std::constructible_from<
                    key_type, KT&&
                > && std::constructible_from<
                    value_type, VT&&
                >
            constexpr item_storage(KT&& key, VT&& value)
                : m_key(std::forward<KT>(key))
                , m_value(std::forward<VT>(value))
                , m_initialized(true)
            {
            }

            template <typename UserTypeT>
                requires reflectable<UserTypeT> && pfr::tuple_size_v<
                    std::remove_reference_t<UserTypeT>
                > == 2 && std::constructible_from<
                    key_type, copy_refs_from_t<
                        UserTypeT&&,
                        pfr::tuple_element_t<
                            0, std::remove_reference_t<UserTypeT>
                        >
                    >
                > && std::constructible_from<
                    value_type, copy_refs_from_t<
                        UserTypeT&&,
                        pfr::tuple_element_t<
                            1, std::remove_reference_t<UserTypeT>
                        >
                    >
                >
            constexpr item_storage(UserTypeT&& user_type)
                : m_key(pfr::get<0>(std::forward<UserTypeT>(user_type)))
                , m_value(pfr::get<1>(std::forward<UserTypeT>(user_type)))
                , m_initialized(true)
            {
            }

            template <typename PairT>
                requires std::constructible_from<
                    key_type, copy_refs_from_t<
                        PairT&&,
                        std::tuple_element_t<0, std::remove_reference_t<PairT>>
                    >
                > && std::constructible_from<
                    value_type, copy_refs_from_t<
                        PairT&&,
                        std::tuple_element_t<1, std::remove_reference_t<PairT>>
                    >
                >
            constexpr item_storage(PairT&& pair)
                : m_key(std::get<0>(std::forward<PairT>(pair)))
                , m_value(std::get<1>(std::forward<PairT>(pair)))
                , m_initialized(true)
            {
            }

            template <typename SelfT>
            constexpr copy_quals_from_t<SelfT&&, key_type> key(this SelfT&& self)
            {
                return self.m_key;
            }

            template <typename SelfT>
            constexpr copy_quals_from_t<SelfT&&, value_type> value(this SelfT&& self)
            {
                return self.m_value;
            }

            constexpr bool initialized() const
            {
                return m_initialized;
            }

        private:
            key_type m_key;
            value_type m_value;
            bool m_initialized = false;
        };
        using storage_type = std::tuple<item_storage<Keys, Values>...>;
        
        static constexpr auto tupled_key_types = hana::make_tuple(hana::type_c<Keys>...);
        static constexpr auto tupled_value_types = hana::make_tuple(hana::type_c<Values>...);

        /* template <size_t... Is>
        static consteval decltype(auto) to_std_tuple(
            hana::basic_range<size_t, */

        static consteval decltype(auto) tupled_index_sequence_for_keys()
        {
            static constexpr auto to_std_tuple = []<size_t... Is>(
                std::index_sequence<Is...>
            ) consteval {
                return std::make_tuple(Is...);
            };
            return to_std_tuple(std::index_sequence_for<Keys...>{});
        }

        static consteval decltype(auto) tupled_indexed_key_types()
        {
            return hana::zip(
                hana::make_range(
                    hana::size_c<0>,
                    hana::size_c<sizeof...(Keys)>
                ), tupled_key_types
            );
        }

        template <typename... Types, typename F, size_t IsLast, size_t... Is>
        static constexpr void for_each_in_tuple_impl(
            const std::tuple<Types...>& tuple, F&& f, std::index_sequence<Is..., IsLast>
        ) {
            // !!! only forward the function for the last element !!!
            (std::invoke(f, std::get<Is>(tuple)), ...);
            std::invoke(std::forward<F>(f), std::get<IsLast>(tuple));
        }

        template <typename... Types, typename F, size_t Is>
        static constexpr void for_each_in_tuple_impl(
            const std::tuple<Types...>& tuple, F&& f, std::index_sequence<Is>
        ) {
            std::invoke(std::forward<F>(f), std::get<Is>(tuple));
        }

        template <typename... Types, typename F>
        static constexpr void for_each_in_tuple_impl(
            const std::tuple<Types...>& tuple, F&& f, std::index_sequence<>
        ) { }
        
        template <typename... Types, typename F>
        static constexpr void for_each_in_tuple(const std::tuple<Types...>& tuple, F&& f)
        {
            for_each_in_tuple_impl(tuple, std::forward<F>(f), std::index_sequence_for<Types...>{});
        }

        template <size_t N, typename... Types, typename F>
        static constexpr void for_each_in_tuple_except(const std::tuple<Types...>& tuple, F&& f)
        {
            for_each_in_tuple_impl(
                tuple, std::forward<F>(f), index_sequence_for_except<N, Types...>{}
            );
        }
    
        //template <typename KeyType>
        //static consteval decltype(auto) indexes_with_key_type()
        //{
        //    return hana::transform(
        //        hana::filter(
        //            tupled_indexed_key_types(),
        //            hana::compose(
        //                hana::equal.to(hana::type_c<KeyType>),
        //                hana::second
        //            )
        //        ), hana::first
        //    );
        //}

        template <size_t I, size_t... Is>
        static consteval decltype(auto) extract_true_indexes(
            const bool (&arr)[1 + sizeof...(Is)], std::index_sequence<I, Is...>
        )
        {
            if constexpr (sizeof...(Is) == 0)
            {
                if constexpr (arr[I])
                    return std::make_tuple(I);
                else
                    return std::tuple{};
            }
            else
            {
                bool remainings[sizeof...(Is)] = { arr[Is]... };
                if (arr[I])
                    return std::tuple_cat(
                        std::make_tuple(I),
                        extract_true_indexes(remainings, std::index_sequence<Is...>{})
                    );
                else
                    return extract_true_indexes(remainings, std::index_sequence<Is...>{});
            }
        }
                

        template <typename KeyType>
        static consteval decltype(auto) indexes_equality_comparable_with_key_type()
        {
            constexpr bool is_equality_comparable[] = {
                std::equality_comparable_with<KeyType, Keys>...
            };
            return extract_true_indexes(is_equality_comparable, std::index_sequence_for<Keys...>{});
        }

        constexpr bool unique_keys()
        {
            bool result = true;
            for_each_in_tuple(
                zip_tuples(tupled_index_sequence_for_keys(), m_storage),
                [&](auto&& pair) {
                    auto [index, item] = pair;
                    for_each_in_tuple(
                        zip_tuples(tupled_index_sequence_for_keys(), m_storage),
                        [&](auto&& other_pair) {
                            auto [other_index, other_item] = other_pair;
                            if (index != other_index && item.key() == other_item.key())
                                result = false;
                        }
                    );
                }
            );
            return result;
        }

    public:
        template <size_t N>
        using key_type = typename nth_type<N, Keys...>::type;

        template <size_t N>
        using value_type = typename nth_type<N, Values...>::type;

        template <size_t N>
        using pair_type = item_storage<key_type<N>, value_type<N>>;

        constexpr simple_map() = delete;

        template <typename... Pairs>
        constexpr simple_map(Pairs&&... pairs)
            : m_storage{std::forward<Pairs>(pairs)...}
        {
            if (!unique_keys())
                throw std::invalid_argument("Keys must be unique");
        }

        template <typename KeyT>
        constexpr decltype(auto) at(KeyT&& key) const
        {
            constexpr auto indexes =
                indexes_equality_comparable_with_key_type<
                    std::remove_cvref_t<KeyT>
                >();
            if constexpr (std::tuple_size_v<decltype(indexes)> == 0)
                throw std::invalid_argument("Key type is not equality comparable with any key type in the map");
            else
            {
                constexpr size_t found_index = this->at_impl(
                    std::forward<KeyT>(key), indexes
                );
                if constexpr (found == -1)
                    throw std::out_of_range("Key not found");
                return std::get<found_index>(m_storage).value();
            }
        }
    
    private:
        storage_type m_storage;
    };

    namespace detail
    {
        namespace test
        {
            consteval bool hana_map_find_test()
            {
                auto map = hana::make_map(
                    hana::make_pair(hana::type_c<int>, 1),
                    hana::make_pair(hana::type_c<float>, 2.0f),
                    hana::make_pair(hana::type_c<double>, 3.0)
                );
                return hana::find(map, hana::type_c<float>) == hana::just(2.0f);
            }
        }
    }
#endif
}

#endif

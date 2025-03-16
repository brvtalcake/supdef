#ifndef SIMPLE_MAP_HPP
#define SIMPLE_MAP_HPP

#include <types.hpp>

namespace supdef
{
    template <typename... Keys, typename... Values>
    class simple_map
    {
        using storage_type = std::tuple<std::pair<Keys, Values>...>;
        
        static constexpr auto tupled_key_types = hana::make_tuple(hana::type_c<Keys>...);
        static constexpr auto tupled_value_types = hana::make_tuple(hana::type_c<Values>...);

        static consteval decltype(auto) tupled_indexed_key_types()
        {
            return hana::zip(
                hana::make_range(
                    hana::size_c<0>,
                    hana::size_c<sizeof...(Keys)>
                ), tupled_key_types
            );
        }

        static constexpr void for_each_in_tuple(std::tuple<Types...> tuple, auto&& f)
        {
            for_each_in_tuple_impl(tuple, f, std::index_sequence_for
        }
    
        template <typename KeyType>
        static consteval decltype(auto) indexes_with_key_type()
        {
            return hana::transform(
                hana::filter(
                    tupled_indexed_key_types(),
                    hana::compose(
                        hana::equal.to(hana::type_c<KeyType>),
                        hana::second
                    )
                ), hana::first
            );
        }

        consteval bool unique_keys()
        {
            bool result = true;
            result &= hana::all_of(
                hana::transform(
                    tupled_key_types,
                    [](auto key_type) {
                        return hana::count(tupled_key_types, key_type) == hana::size_c<1>;
                    }
                ),
                hana::id
            );
            hana::for_each
        }
    public:
        template <size_t N>
        using pair_type = std::tuple_element_t<N, storage_type>;

        template <size_t N>
        using key_type = std::tuple_element_t<N, storage_type>::first_type;

        template <size_t N>
        using value_type = std::tuple_element_t<N, storage_type>::second_type;

        constexpr simple_map() = default;

        template <typename... Pairs>
        constexpr simple_map(Pairs&&... pairs)
            : storage{std::forward<Pairs>(pairs)...}
        {
        }

        template <typename... Pairs>
        constexpr simple_map(std::tuple<Pairs...>&& pairs)
            : storage(std::move(pairs))
        {
        }
    
    private:
        storage_type storage;
    };
}

#endif

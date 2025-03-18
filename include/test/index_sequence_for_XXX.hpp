#ifdef __INTELLISENSE__
    #include <types.hpp>
#endif

namespace supdef
{
    // index_sequence_for_except
    static_assert(
        std::same_as<
            index_sequence_for_except<1, int, short, long>,
            std::index_sequence<0, 2>
        >,
        "index_sequence_for_except<1, int, short, long> is not std::index_sequence<0, 2>"
    );
    static_assert(
        std::same_as<
            index_sequence_for_except<0, int>,
            std::index_sequence<>
        >,
        "index_sequence_for_except<0, int> is not std::index_sequence<>"
    );

    // index_sequence_for_tuple
    static_assert(
        std::same_as<
            index_sequence_for_tuple<std::tuple<int, short, long>>,
            std::index_sequence<0, 1, 2>
        >,
        "index_sequence_for_tuple<std::tuple<int, short, long>> is not std::index_sequence<0, 1, 2>"
    );
    static_assert(
        std::same_as<
            index_sequence_for_tuple<std::tuple<>>,
            std::index_sequence<>
        >,
        "index_sequence_for_tuple<std::tuple<>> is not std::index_sequence<>"
    );
}
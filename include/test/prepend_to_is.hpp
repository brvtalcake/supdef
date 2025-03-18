
namespace supdef
{
    static_assert(
        std::same_as<
            prepend_to_index_sequence_t<1234, std::index_sequence<>>,
            std::index_sequence<1234>
        >
    );
    static_assert(
        std::same_as<
            prepend_to_index_sequence_t<1234, std::index_sequence<5678>>,
            std::index_sequence<1234, 5678>
        >
    );
    static_assert(
        std::same_as<
            prepend_to_index_sequence_t<1234, std::index_sequence<5678, 91011>>,
            std::index_sequence<1234, 5678, 91011>
        >
    );
}

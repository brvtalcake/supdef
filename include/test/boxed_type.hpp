namespace supdef
{
    static_assert(
        std::same_as<
            boxed_type_t<int*>, int
        >, "boxed_type_t<int*> is not int"
    );
    static_assert(
        std::same_as<
            boxed_type_t<boost::local_shared_ptr<int>>, int
        >, "boxed_type_t<boost::local_shared_ptr<int>> is not int"
    );
    static_assert(
        std::same_as<
            boxed_type_t<int[]>, int
        >, "boxed_type_t<int[]> is not int"
    );
    static_assert(
        std::same_as<
            boxed_type_t<int[5]>, int
        >, "boxed_type_t<int[5]> is not int"
    );

    static_assert(
        std::same_as<
            boxed_type_t<const int*>, const int
        >, "boxed_type_t<const int*> is not const int"
    );
    static_assert(
        std::same_as<
            boxed_type_t<boost::local_shared_ptr<const int>>, const int
        >, "boxed_type_t<boost::local_shared_ptr<const int>> is not const int"
    );
    static_assert(
        std::same_as<
            boxed_type_t<const int[]>, const int
        >, "boxed_type_t<const int[]> is not const int"
    );
    static_assert(
        std::same_as<
            boxed_type_t<const int[5]>, const int
        >, "boxed_type_t<const int[5]> is not const int"
    );

    static_assert(
        std::same_as<
            boxed_type_t<const int* const>, const int
        >, "boxed_type_t<const int*> is not const int"
    );
    static_assert(
        std::same_as<
            boxed_type_t<boost::local_shared_ptr<const int> const>, const int
        >, "boxed_type_t<boost::local_shared_ptr<const int>> is not const int"
    );

    static_assert(
        std::same_as<
            boxed_type_t<volatile int* const volatile&>, volatile int
        >, "boxed_type_t<volatile int* const volatile&> is not volatile int"
    );
    static_assert(
        std::same_as<
            boxed_type_t<boost::local_shared_ptr<volatile int> const volatile&>, volatile int
        >, "boxed_type_t<boost::local_shared_ptr<volatile int> const volatile&> is not volatile int"
    );
}
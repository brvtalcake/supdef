namespace supdef
{
    namespace detail
    {
        static_assert(
            decltype(
                min_arity_of_fn([](int, int) { return 0; })
            )::value == 2,
            "min_arity_of_fn is broken"
        );
        static_assert(
            decltype(
                min_arity_of_fn([](int, int, ...) { return 0; })
            )::value == 2,
            "min_arity_of_fn is broken"
        );
        static_assert(
            decltype(
                min_arity_of_fn(BOOST_HOF_LIFT([](int, int) { return 0; }))
            )::value == 2,
            "min_arity_of_fn is broken"
        );
        static_assert(
            decltype(
                min_arity_of_fn(BOOST_HOF_LIFT([](int, int, ...) { return 0; }))
            )::value == 2,
            "min_arity_of_fn is broken"
        );

        static_assert(
            decltype(
                max_arity_of_fn(hana::size_c<2>, [](int, int) { return 0; })
            )::value == 2,
            "max_arity_of_fn is broken"
        );
        static_assert(
            decltype(
                max_arity_of_fn(hana::size_c<2>, [](int, int, ...) { return 0; })
            )::value == 100,
            "max_arity_of_fn is broken"
        );
        static_assert(
            decltype(
                max_arity_of_fn(hana::size_c<2>, BOOST_HOF_LIFT([](int, int) { return 0; }))
            )::value == 2,
            "max_arity_of_fn is broken"
        );
        static_assert(
            decltype(
                max_arity_of_fn(hana::size_c<2>, BOOST_HOF_LIFT([](int, int, ...) { return 0; }))
            )::value == 100,
            "max_arity_of_fn is broken"
        );
    }

    static_assert(
        arity([]() { return 0; }).value == std::pair<size_t, size_t>{ 0, 0 },
        "arity is broken"
    );
    static_assert(
        arity_of([](int, int) { return 0; }).value == std::pair<size_t, size_t>{ 2, 2 },
        "arity_of is broken"
    );
    static_assert(
        arity_of([](int, int, ...) { return 0; }).value == std::pair<size_t, size_t>{ 2, (size_t)-1 },
        "arity_of is broken"
    );
    static_assert(
        arity_of([](int, int, bool = true) { return 0; }).value == std::pair<size_t, size_t>{ 2, 3 },
        "arity_of is broken"
    );
    static_assert(
        arity_of(BOOST_HOF_LIFT([]() { return 0; })).value == std::pair<size_t, size_t>{ 0, 0 },
        "arity_of is broken"
    );
    static_assert(
        arity_of(BOOST_HOF_LIFT([](int, int) { return 0; })).value == std::pair<size_t, size_t>{ 2, 2 },
        "arity_of is broken"
    );
    static_assert(
        arity_of(BOOST_HOF_LIFT([](int, int, ...) { return 0; })).value == std::pair<size_t, size_t>{ 2, (size_t)-1 },
        "arity_of is broken"
    );
    static_assert(
        arity_of(BOOST_HOF_LIFT([](int, int, bool = true) { return 0; })).value == std::pair<size_t, size_t>{ 2, 3 },
        "arity_of is broken"
    );
}
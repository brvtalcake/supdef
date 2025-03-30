#ifdef __INTELLISENSE__
    #include <types.hpp>
#endif

namespace supdef
{
    namespace test
    {
        consteval bool test_typelist()
        {
            supdef::typelist<int, char, double> tl{};
            bool ret = true;
            for (const auto& var : tl)
            {
                std::visit(
                    [&](auto&& arg) {
                        if (!ret)
                            return;

                        using T = std::decay_t<decltype(arg)>;
                        ret &= supdef::specialization_of<T, std::pair>;
                        if (!ret)
                            return;

                        auto ic = arg.first;
                        auto type = arg.second;
                        ret &= (ic.value < 3);
                        if (!ret)
                            return;

                        if constexpr (ic == 0)
                            ret &= (hana::type_c<int> == type);
                        else if constexpr (ic == 1)
                            ret &= (hana::type_c<char> == type);
                        else if constexpr (ic == 2)
                            ret &= (hana::type_c<double> == type);
                        else
                            ret = false;
                        
                        return;
                    },
                    var
                );
            }

            return ret;
        }
    }

    static_assert(std::bool_constant<test::test_typelist()>::value);
}
#include <detail/globals.hpp>
#include <init.hpp>
#include <types.hpp>
#include <interpreter.hpp>

GLOBAL_DEF_START(supdef::unused_t, set_default_boost_multiprecision_config_values)
    supdef::bigint::default_precision(supdef::interpreter::boost_multiprecision_default_precision);
    supdef::bigint::default_variable_precision_options(supdef::interpreter::boost_multiprecision_default_vpo);
    supdef::bigfloat::default_precision(supdef::interpreter::boost_multiprecision_default_precision);
    supdef::bigfloat::default_variable_precision_options(supdef::interpreter::boost_multiprecision_default_vpo);
    supdef::bigcomplex::default_precision(supdef::interpreter::boost_multiprecision_default_precision);
    supdef::bigcomplex::default_variable_precision_options(supdef::interpreter::boost_multiprecision_default_vpo);
    supdef::bigdecimal::default_precision(supdef::interpreter::boost_multiprecision_default_precision);
    supdef::bigdecimal::default_variable_precision_options(supdef::interpreter::boost_multiprecision_default_vpo);
GLOBAL_DEF_END_BASIC
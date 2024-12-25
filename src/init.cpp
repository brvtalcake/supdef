#include <init.hpp>

#if STATIC_INITIALIZER_ALLOCATION
supdef::init::array_type supdef::init::s_init_array;
size_t supdef::init::s_init_array_counter = 0;
#else
std::list<supdef::init::list_value_type, supdef::init::alloc_type> supdef::init::s_init_list;
#endif

supdef::init::client::client(client_func_type f) noexcept
    : m_func(f)
{
    global_initializers.register_init(*this);
}

supdef::init::client::~client()
{ }

void supdef::init::client::operator()() const noexcept
{
    m_func();
}

void supdef::init::register_init(const client& c) noexcept
{
#if STATIC_INITIALIZER_ALLOCATION
    s_init_array[s_init_array_counter++] = std::addressof(c);
#else
    s_init_list.push_back(std::ref(c));
#endif
}

void supdef::init::operator()() noexcept
{
#if STATIC_INITIALIZER_ALLOCATION
    for (size_t i = 0; i < s_init_array_counter; ++i)
        (*s_init_array[i])();
#else
    for (const auto& c : s_init_list)
        c.get()();
#endif
}
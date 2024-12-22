#include <init.hpp>

std::list<supdef::init::client, supdef::init::alloc_type> supdef::init::s_init_list;

supdef::init::client::client(client_func_type f) noexcept
    : m_func(f)
{
    global_initializers.register_init(*this);
}

supdef::init::client::~client()
{ }

void supdef::init::client::operator()() noexcept
{
    m_func();
}

void supdef::init::register_init(client c) noexcept
{
    s_init_list.push_back(c);
}

void supdef::init::operator()() noexcept
{
    for (auto& c : s_init_list)
        c();
}
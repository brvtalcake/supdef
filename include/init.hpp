#ifndef INIT_HPP
#define INIT_HPP

#include <list>

#include <static_alloc.hpp>

#undef  INIT_PRIORITY
#define INIT_PRIORITY(prio)  [[__gnu__::__init_priority__(prio)]]

#undef  INIT_PRIO_HIGH
#undef  INIT_PRIO_LOW
#define INIT_PRIO_HIGH 101
#define INIT_PRIO_LOW 65535

#undef  INIT_PRIO_HIGHER
#undef  INIT_PRIO_LOWER
#define INIT_PRIO_HIGHER(baseprio, diff) ((baseprio) - (diff))
#define INIT_PRIO_LOWER(baseprio, diff)  ((baseprio) + (diff))

#undef  INIT_CLASS_PRIO
#define INIT_CLASS_PRIO INIT_PRIORITY(INIT_PRIO_HIGHER(INIT_PRIO_LOW, 1))

#undef  INIT_CLIENT_CLASS_PRIO
#define INIT_CLIENT_CLASS_PRIO INIT_PRIORITY(INIT_PRIO_LOW)

#undef  MAX_INIT_CLIENTS
#define MAX_INIT_CLIENTS 32

namespace supdef
{
    class init
    {
    public:
        using client_func_type = void (*)(void) noexcept;
        class client
        {
        public:
            client(client_func_type f) noexcept;
            ~client();

            void operator()() const noexcept;
        
        private:
            client_func_type m_func;
        };

        init() = default;
        ~init() = default;

        void register_init(const client& c) noexcept;
        void operator()() noexcept;
    
    protected:
#if STATIC_INITIALIZER_ALLOCATION
        using array_value_type = const client*;
        using array_type = std::array<array_value_type, MAX_INIT_CLIENTS>;
        static array_type s_init_array;
        static size_t s_init_array_counter;
#else
        using list_value_type = std::reference_wrapper<const client>;
        using alloc_type = std::allocator<list_value_type>;
        static std::list<list_value_type, alloc_type> s_init_list;
#endif
    };

    extern init global_initializers;
}


#endif

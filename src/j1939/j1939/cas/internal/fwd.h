#pragma once

#include <estd/cstdint.h>

#if __cpp_lib_concepts
#include <concepts>
#endif

namespace embr { namespace j1939 { namespace internal {

#if __cpp_concepts
namespace concepts {

template <class T>
concept RandomGenerator = requires(T t)
{
#if __cpp_lib_concepts
    { t.get() } -> std::convertible_to<unsigned>;
#else
    t.get();
#endif
};

template <class T>
concept AddressManager = requires(T t)
{
    // Address seen over transport that isn't us
    t.encountered(uint8_t{});

#if __cpp_lib_concepts
    { t.depleted() } -> std::convertible_to<uint8_t>;
#endif
};

}
#endif

}}}

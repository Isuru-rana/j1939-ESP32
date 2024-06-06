#pragma once

namespace embr { namespace j1939 { namespace internal {

#if __cpp_concepts
template <class T>
concept TESTER = requires(T t)
{
    t.hello();
};
#endif

}}}
#include "j1939/pdu/traits.h"
#include "j1939/state-machines/transport_protocol.hpp"

// Clang 14 is sensitive to this:
// https://stackoverflow.com/questions/8452952/c-linker-error-with-class-static-constexpr

namespace embr { namespace j1939 {

#ifndef __cpp_inline_variables

namespace internal {

constexpr uint8_t address_type_traits_base::global;
constexpr uint8_t address_type_traits_base::null;

}

namespace sm { namespace tp { inline namespace v0 {

using mst = const estd::chrono::milliseconds;

mst base::timeouts::bam;
mst base::timeouts::T1;
mst base::timeouts::T2;
mst base::timeouts::T3;
mst base::timeouts::T4;
mst base::timeouts::Th;

}}}

#endif

}}


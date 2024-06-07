#include "j1939/pdu/traits.h"

// Clang 14 is sensitive to this:
// https://stackoverflow.com/questions/8452952/c-linker-error-with-class-static-constexpr

namespace embr { namespace j1939 {

namespace internal {

#ifndef __cpp_inline_variables
constexpr uint8_t address_type_traits_base::global;
constexpr uint8_t address_type_traits_base::null;
#endif

}

}}

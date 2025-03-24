#pragma once

#include "enum.h"

#if __cpp_lib_concepts
#include <concepts>
#endif


namespace embr { namespace can {

#if __cpp_concepts
// TODO: Enforce frame_traits (& transport_traits) signature here
#endif

template <class Frame>
struct frame_traits;

// DEBT: This belongs elsewhere up in EMBR or ESTD
// CLion is throwing a serious warning fit
#if __cplusplus >= 201703L
#define ATTR_NODISCARD      [[nodiscard]]
#define ATTR_FALLTHROUGH    [[fallthrough]]
#else
#define ATTR_NODISCARD
#define ATTR_FALLTHROUGH
#endif


}}

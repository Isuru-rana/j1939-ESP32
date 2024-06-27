#pragma once

#include "enum.h"

namespace embr { namespace can {

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

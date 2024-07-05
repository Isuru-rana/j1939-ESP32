#pragma once

#include "result.h"

// DEBT: Consider putting this into embr proper.  Look into embr::coap state machine support for consolidation
// ideas

// entire file is
// EXPERIMENTAL

namespace embr { namespace j1939 { namespace sm { inline namespace v0 {

class base
{
public:
    template <class ...Args>
    static constexpr result process(Args&&...)
    {
        return result::ignore();
    }
};

}}}}

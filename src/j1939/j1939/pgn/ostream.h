#pragma once

#include <estd/iosfwd.h>
#include <estd/locale.h>

// DEBT: Only need this directly here for 'estd::hex' would be nice if that
// was available in iosfwd instead
#include <estd/ostream.h>

#include "fwd.h"
#include "../pdu/ostream/fwd.h"
#include "../pdu/ostream.h"
#include "../pgn/traits.h"
#include "../data_field/fwd.h"

namespace embr { namespace j1939 {

// DEBT: All this stuff seems better suited to a pdu specific area, not pgn

namespace internal {

// DEBT: Not great naming
// Helper to reduce code bloat slightly (this has better chance of not inlining)
template <class PduHeader, class Streambuf, class Base>
void out_helper(const char* abbrev,
    const PduHeader& can_id,
    estd::detail::basic_ostream<Streambuf, Base>& out)
{
    out << abbrev << ' ';

    // Outputs PDU header portion
    out << estd::hex << can_id << ' ';
}

// DEBT: Sloppy
template <class PduHeader, class Payload, class Streambuf, class Base>
void out_pdu_helper(estd::detail::basic_ostream<Streambuf, Base>& out,
    uint32_t pgn, const PduHeader& can_id, const Payload& payload)
{
    out << pgn << ' ' << estd::hex << can_id << ' ';

    //using data_field_type = estd::remove_cvref_t<decltype(payload)>;

    write_raw_payload(out, payload);
    //payload_put_base<typename data_field_type::container_type>{payload}(out);
}

template <pgns pgn>
struct pgn_put<pgn, estd::enable_if_t<pgn::traits<pgn>::is_specialized> > :
    estd::internal::ostream_functor_tag
{
    const pdu<pgn>& pdu_;

    using container_type = typename pdu<pgn>::container_type;

    using traits = pgn::traits<pgn>;

    constexpr pgn_put(const pdu<pgn>& p) : pdu_{p} {}

    template <class Streambuf, class Base>
    void operator()(estd::detail::basic_ostream<Streambuf, Base>& out) const
    {
        // DEBT: Consolidate this into a helper function to avoid code bloat
#if FEATURE_EMBR_J1939_OSTREAM_PGN_ABBREV
        out_helper(traits::abbrev(), pdu_.can_id(), out);
#else
        // Turns out that due to code bloat it takes MORE code to do this
        // uint32_t output on AVR than the abbrev above.  That will likely
        // change with a helper function though
#error Unsupported
        out << (uint32_t) pgn << ' ';

        // Outputs PDU header portion
        out << estd::hex << pdu_.can_id() << ' ';
#endif

        const auto& payload = pdu_.payload();

#if FEATURE_EMBR_J1939_OSTREAM_FULL_PAYLOAD
        payload_put<pgn, container_type>{payload}(out);
#else
        // Saves over 1k of code space easily
        using data_field_type = estd::remove_cvref_t<decltype(payload)>;

        payload_put_base<typename data_field_type::container_type>{payload}(out);
#endif
    }
};

// NOTE: Not well tested
template <pgns pgn>
struct pgn_put<pgn, estd::enable_if_t<!pgn::traits<pgn>::is_specialized> > :
    estd::internal::ostream_functor_tag
{
    // DEBT: Do a more generic pdu if we can
    const pdu<pgn>& pdu_;

    constexpr pgn_put(const pdu<pgn>& p) : pdu_{p} {}

    template <class Streambuf, class Base>
    void operator()(estd::detail::basic_ostream<Streambuf, Base>& out) const
    {
        // Keep an eye on AVR code spew
        out_pdu_helper(out, (uint32_t)pgn, pdu_.can_id(), pdu_.payload());
    }
};


}

}

}

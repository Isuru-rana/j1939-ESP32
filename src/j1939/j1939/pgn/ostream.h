#pragma once

#include <estd/iosfwd.h>
#include <estd/locale.h>

// DEBT: Only need this directly here for 'estd::hex' would be nice if that
// was available in iosfwd instead
#include <estd/ostream.h>

#include "fwd.h"
#include "../pdu/ostream.h"
#include "../pgn/traits.h"
#include "../data_field/fwd.h"

namespace embr { namespace j1939 {

// DEBT: All this stuff seems better suited to a pdu specific area, not pgn

template <pgns pgn, class TStreambuf, class TBase>
constexpr estd::detail::basic_ostream<TStreambuf, TBase>& operator <<(
    estd::detail::basic_ostream<TStreambuf, TBase>& out,
    const pdu<pgn>& p)
{
    return out << embr::put_pdu(p);
}


namespace internal {

template <class TContainer>
struct payload_put_base : estd::internal::ostream_functor_tag
{
    using payload_type = internal::data_field_base<TContainer>;
    const payload_type& payload;

    constexpr explicit payload_put_base(const payload_type& payload) : payload{payload} {}

    template <class TStreambuf, class TBase>
    void operator()(estd::detail::basic_ostream<TStreambuf, TBase>& out) const
    {
        for(unsigned v : payload)   out << v << ' ';
    }
};

template <pgns pgn, class TContainer>
struct payload_put : payload_put_base<TContainer>
{
    constexpr explicit payload_put(const data_field<pgn>& payload) :
        payload_put_base<TContainer>{payload} {}
};


// DEBT: Not great naming
// Helper to reduce code bloat slightly (this has better change of not inlining)
template <class TPduHeader, class TStreambuf, class TBase>
void out_helper(const char* abbrev,
    const TPduHeader& can_id,
    estd::detail::basic_ostream<TStreambuf, TBase>& out)
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

    using data_field_type = estd::remove_cvref_t<decltype(payload)>;

    payload_put_base<typename data_field_type::container_type>{payload}(out);
}

template <pgns pgn>
#if FEATURE_EMBR_J1939_NO_TRAITS_WRAPPER
struct pgn_put<pgn, estd::enable_if_t<pgn::traits<pgn>::is_specialized> > :
#else
struct pgn_put<pgn, void> :
#endif
    estd::internal::ostream_functor_tag
{
    const pdu<pgn>& pdu_;

#if FEATURE_EMBR_J1939_NO_TRAITS_WRAPPER
    using traits = pgn::traits<pgn>;
#else
    using traits = traits_wrapper<pgn>;
#endif

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
        payload_put<pgn>{payload}(out);
#else
        // Saves over 1k of code space easily
        using data_field_type = estd::remove_cvref_t<decltype(payload)>;

        payload_put_base<typename data_field_type::container_type>{payload}(out);
#endif
    }
};

#if FEATURE_EMBR_J1939_NO_TRAITS_WRAPPER
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
#endif


}

}

template <j1939::pgns pgn>
constexpr j1939::internal::pgn_put<pgn> put_pdu(const j1939::pdu<pgn>& pdu_)
{
    return { pdu_ };
}

}

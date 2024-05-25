#pragma once

#include <can/platform/esp-idf/transport.hpp>

#include <j1939/pdu.h>
#include <j1939/ca.h>

// non blocking
using transport_type = embr::can::esp_idf::twai_transport<false>;

class component_identification_ca : public embr::j1939::impl::controller_application_base
{
    template <pgns pgn>
    using pdu = const embr::j1939::pdu<pgn>;

public:
    template <class Transport, pgns pgn>
    static constexpr bool process_incoming(Transport& t, pdu<pgn> p) { return false; }

    template <class Transport>
    bool process_incoming(Transport& t, pdu<pgns::request>& p);

    template <class Transport>
    bool process_outgoing(Transport& t);
};
#pragma once

#include <j1939/pdu.h>
#include <j1939/ca.h>

class component_identification_ca : public embr::j1939::cs::v1::base
{
    using base_type = embr::j1939::cs::v1::base;

public:
    struct policy_type : base_type::policy_type
    {
        using whitelist = pgn_list<pgns::request>;
    };

    using base_type::process_incoming;

    template <class Transport>
    bool process_incoming(Transport& t, const pdu<pgns::request>& p);

    template <class Transport>
    bool process_outgoing(Transport& t);
};
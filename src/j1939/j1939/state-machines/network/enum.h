/**
 *
 * References:
 *
 * 1. J1939-81 (draft MAY2003)
 * 2. J1939-21 (DEC2006)
 * 3. AddressResolution.md v0.1
 */
#pragma once

namespace embr { namespace j1939 { namespace sm { inline namespace v1 {

struct network_enum
{
    // DEBT: Upgrade this to embr 'service' architecture
    enum class states
    {
        unstarted,
        requesting,         ///< Request for address_claimed [1] A5, A6, A7 Initialize
        claiming,           ///< Address Claim - emit and wait
        claimed,            ///< Address Claim success without contention
        claim_failed,       ///< No possibility of acquiring address

        // EXPERIMENTAL
        //bus_error           ///< Unless address != null, all bets are off and similar to 'unstarted'
    };


    // Guidance from [1] Appendix D: State Transition Diagrams
    enum class substates
    {
        unstarted,

        // generic reused states
        sending,            ///< Indicate transport is in process of emitting something (see 'states' for what)
        waiting,            ///< Indicate main state has done what it can, and now waiting for response traffic
        //send_error,

        // requesting state

        /// Waiting period (1250ms) after we emit a request for address claim
        request_waiting, // = waiting,  // DEBT: scheduled_claiming doesn't quite disambiguate enough here

        // claiming state

        /// Waiting period (250ms) after we emit a claim address
        claim_waiting,  // = waiting,
        contending,         ///< Evaluation period after we receive a contending address
        claim_send_error,   ///< Same as 'waiting' but bus/send error occurred [3] 1.1.4
        reclaim_waiting,    ///< Waiting period of 0-153ms preceding re-transmit of claim [1] 4.4.4.3
        bus_off,            ///< FIX: These bus_off states appear to be in conflict with reclaim_waiting
        bus_off_recover,

        // claimed state
        expired,            ///< Claim waiting period expired with no incident, meaning we succeeded
        elapsed = expired,

        // claim failed state
        failed,             ///< Root substate when we've given up trying to get SA
        cannot_claim_waiting,   ///< Waiting period of 0-153ms preceding the emit of "cannot claim" [1] 4.2.2.3
    };
};

}

inline namespace v0 {

// DEBT: Doesn't play nice in an 'internal' namespace
// Convenience class useful for those who externally want to track & compare states
class network_states : public v1::network_enum
{
protected:
    states state_ = states::unstarted;
    substates substate_ = substates::unstarted;

    void state(states s, substates ss)
    {
        state_ = s;
        substate_ = ss;
    }

public:
    constexpr states state() const { return state_; }
    constexpr substates substate() const { return substate_; }
};

class network_cached : public network_states
{
public:
    void state(states v) { state_ = v; }
    void state(substates v) { substate_ = v; }

    bool state(const network_states& v)
    {
        if(state_ == v.state() && substate_ == v.substate()) return false;

        state_ = v.state();
        substate_ = v.substate();
        return true;
    }
};

}

}}}

namespace embr { namespace j1939 {

const char* to_string(j1939::sm::v1::network_enum::states v);
const char* to_string(j1939::sm::v1::network_enum::substates v);

}}

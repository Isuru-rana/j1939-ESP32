#pragma once

#include <embr/scheduler.h>

#include <j1939/cas/network.hpp>
#include <j1939/cas/internal/prng_address_manager.h>

#include "transport.h"


using scheduler_impl_type =
    embr::internal::scheduler::impl::Function<estd::chrono::freertos_clock::time_point>;

using scheduler_type = embr::internal::layer1::Scheduler<10, scheduler_impl_type>;


using nca_type = embr::j1939::impl::network_ca<transport_type,
    scheduler_type,
    embr::j1939::internal::prng_address_manager>;

extern nca_type nca;

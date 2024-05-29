#include <estd/thread.h>

#include <embr/scheduler.hpp>

using namespace estd::chrono_literals;

void init_console();

using scheduler_impl_type =
    embr::internal::scheduler::impl::Function<estd::chrono::freertos_clock::time_point>;

using scheduler_type = embr::internal::layer1::Scheduler<10, scheduler_impl_type>;

scheduler_type scheduler;

extern "C" void app_main(void)
{
    init_console();

    for(;;)
    {
        estd::this_thread::sleep_for(50ms);
        scheduler.process();
    }
}

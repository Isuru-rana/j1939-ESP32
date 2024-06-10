#include "test-data.h"

#include <estd/tuple.h>
#include <estd/internal/variadic.h>

#include <j1939/cs/base.h>

using namespace embr;

// EXPERIMENTAL
estd::tuple<j1939::cs::v1::base, j1939::cs::v1::base> t1;

TEST_CASE("controller subsystems")
{
#if __cpp_fold_expressions
#endif
}
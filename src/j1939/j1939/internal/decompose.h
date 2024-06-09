#pragma once

#if __cpp_fold_expressions

#include "../spn/fwd.h"
#include "../data_field/base.hpp"

namespace embr::j1939::internal { inline namespace v1 {

// Very clever constexpr int to string, from
// https://stackoverflow.com/questions/6713420/c-convert-integer-to-string-at-compile-time
template<std::intmax_t N>
class to_string_t {

    constexpr static auto buflen() noexcept {
        unsigned int len = N > 0 ? 1 : 2;
        for (auto n = N; n; len++, n /= 10);
        return len;
    }

    char buf[buflen()] = {};

public:
    constexpr to_string_t() noexcept {
        auto ptr = buf + buflen();
        *--ptr = '\0';

        if (N != 0) {
            for (auto n = N; n; n /= 10)
                *--ptr = "0123456789"[(N < 0 ? -1 : 1) * (n % 10)];
            if (N < 0)
                *--ptr = '-';
        } else {
            buf[0] = '0';
        }
    }

    constexpr operator const char *() const { return buf; }
};

template<std::intmax_t N>
constexpr to_string_t<N> to_string;


template <j1939::spns s, class Container, class F>
bool decompose(const j1939::internal::data_field_base<Container>& d, F&& f)
{
    using traits = j1939::spn::traits<s>;

    auto v = d.template get<s>();

    f(traits{}, v);

    return true;
}

template <j1939::spns ...spns, class Container, class F>
void decompose(estd::variadic::values<j1939::spns, spns...>,
    const j1939::internal::data_field_base<Container>& d, F&& f)
{
    (... && decompose<spns>(d, std::forward<F>(f)));
}


// Assess spns of this pgn and iterate one over the other via f
template <j1939::pgns pgn, class Container, class F>
void decompose(const j1939::data_field<pgn, Container>& d, F&& f)
{
    using traits = j1939::pgn::traits<pgn>;
    using spns = typename traits::spns;

    decompose(spns{}, d, std::forward<F>(f));
}

}}

#endif

# Traits

> NOTE: These examples use embedded-friendly `estd::basic_ostream`.  Don't be afraid of
> the `out` reference!

## Traits: single PGN

```c++
template <pgns pgn>
void print_pgn_name()
{
    out << embr::j1939::pgn::traits<pgn>::name() << endl;
}

```


## Traits: multiple SPNs (fold expression)


```c++
template <spns spn>
void print_spn_name()
{
    out << embr::j1939::spn::traits<spn>::name() << endl;
}
```

## Traits: multiple SPNs (estd::variadic)

Since c++11 doesn't have fold expressions, a different approach is required
to compile-time iterate.  Fortunately, `estd::variadic::values` helps with this.

```c++
template <spns spn>
void print_spn_name()
{
    out << embr::j1939::spn::traits<spn>::name() << endl;
}

template <pgns pgn>
void print_spn_names()
{
    using pgn_spns = embr::j1939::pgn::traits<pgn>::spns;
    
    out << embr::j1939::spn::traits<spn>::name() << endl;
}
```

See TBD for estd documentation on the subject

# Dispatch

What amounts to a tuned switch statement, the dispatch mechanism is a convenient way to translate a run time PGN# into a compile time PDU.

(((be careful, Dispatch can produce a lot of code spew)))

## Dispatch: to_string

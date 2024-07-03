# Traits

## Traits: single

```c++
template <pgns pgn>
void print_pgn_name()
{
    cout << embr::j1939::pgn::traits<pgn>::name() << endl;
}

```


## Traits: multiple (fold expression)


```
```

## Traits: multiple (estd::variadic)

Since c++11 doesn't have fold expressions, a different approach is required
to compile-time iterate.  Fortunately, `estd::variadic::values` helps with this.

```
```

See TBD for estd documentation on the subbject

# Dispatch

What amounts to a tuned switch statement, the dispatch mechanism is a convenient way to translate a run time PGN# into a compile time PDU.

(((be careful, Dispatch can produce a lot of code spew)))

## Dispatch: to_string

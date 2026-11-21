# `slow` C++ Template

`<slow>` provides common competitive programming macros and small helpers. Include `<bits/stdc++.h>` before it. It can be used with standard streams, or after `<fast>` to use the custom buffered I/O implementation.

```cpp
#include <bits/stdc++.h>
#include <fast> // Optional; omit this line to use std::cin and std::cout.
#include <slow>
```

When `<fast>` is included first, `<slow>` detects its `cin` and `cout` macros and provides vector and pair input/output operators for `fast_io::Input` and `fast_io::Output`.

## Loop and container macros

| Macro | Expansion / purpose |
| --- | --- |
| `re(i, n)` | Loop `i` from `0` to `n - 1`. |
| `re(i, l, r)` | Loop `i` from `l` to `r - 1`. |
| `re(i, l, r, step)` | Loop `i` from `l` to `r - 1`, adding `step`. |
| `repo(x, range)` | Range-based loop with `auto x`. |
| `all(x)` / `rall(x)` | Forward / reverse iterator pair. |
| `bg(x)` / `ed(x)` | Begin / end iterators. |
| `sz(x)` | Container size cast to `int`. |
| `vc` | Shorthand for `vector`. |
| `pb(x)` | Shorthand for `push_back(x)`. |
| `nl` | Output a newline. |
| `out(x)` / `outs(x)` / `outl(x)` | Output without a separator / with a trailing space / with a newline. |
| `outt(x)` | Output the source spelling of `x`. |

`sz` uses `int`, so it is not suitable when container sizes can exceed `INT_MAX`. The loop macros are intended for simple expressions; avoid arguments with side effects because macro arguments may be evaluated more than once.

## Helpers and debug

`chmin(value, candidate)` and `chmax(value, candidate)` update the first argument when the candidate is smaller or larger, respectively, and return whether it changed.

With `LOCAL` defined, `<slow>` includes the debug printer, struct printer, and recursion tracer. Without `LOCAL`, `debug`, `DEBUG_STRUCT`, and trace macros expand to harmless no-op expressions. See [Debugging Utilities](debugging.md) for supported values and examples.

The stream operators read vectors element-by-element and pairs in member order. They write vectors with a trailing space after each element and pairs separated by one space.

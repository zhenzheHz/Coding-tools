# Debugging Utilities

## Include

Include `<bits/stdc++.h>` before these headers. They provide debug printing, custom struct formatting, and recursion tracing:

```cpp
#include <bits/stdc++.h>
#include <debug_headers/debug.hpp>
#include <debug_headers/struct_debug.hpp>
#include <debug_headers/recursion_trace.hpp>
```

When `ONLINE_JUDGE` is defined, `debug` and the trace macros expand to no-ops. `library/slow` includes these headers automatically under `LOCAL` and supplies no-op macros otherwise.

## `debug(...)`

Print one or more expressions and their source names to `stderr`. It supports arithmetic and enum values, strings, pointers, pairs, tuple-like values, bitsets, and iterable containers. Nested two-dimensional vectors are formatted across lines. For a custom type, register its members with `DEBUG_STRUCT` at global scope after declaring the type:

```cpp
struct Point {
    int x, y;
};
DEBUG_STRUCT(Point, x, y)

int main() {
    std::vector<Point> points{{1, 2}, {3, 4}};
    debug(points);
}
```

`DEBUG_STRUCT` supports up to 16 member names. Unsupported types produce a compile-time error; add a registration for their fields before printing them.

## Recursion tracing

`TRACE_ENTER(args...)` starts a trace scope for the current function, prints the function name and arguments, and increases the thread-local trace depth. `TRACE_SCOPE` is an alias. The scope prints a closing brace when it exits.

```cpp
int sum_to(int n) {
    TRACE_ENTER(n);
    if (n == 0) {
        TRACE_RETURN(0);
        return 0;
    }
    int result = n + sum_to(n - 1);
    TRACE_RETURN(result);
    return result;
}
```

`TRACE_RETURN(value)` only logs the value; write the actual `return` statement separately. `TRACE_DEPTH()` returns the current nesting depth. Traces are written to `stderr` and are disabled when `ONLINE_JUDGE` is defined. Use one `TRACE_ENTER` per function scope.

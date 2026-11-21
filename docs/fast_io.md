# Fast I/O

## Include and setup

Include the standard headers first, then `<fast>`. The header uses C and C++ library facilities but does not include them itself. It requires C++17 for inline global objects.

```cpp
#include <bits/stdc++.h>
#include <fast>

int main() {
    int n;
    if (!(cin >> n)) return 0;
    cout << n << '\n';
}
```

Including `<fast>` defines global `cin` and `cout` macros that refer to `fast_io::in` and `fast_io::out`. They replace the standard stream names in subsequent code, so include `<fast>` after other headers and do not use `std::cin` or `std::cout` afterward.

## Input

The input object reads whitespace-separated values from `stdin`. It has a 64 KiB buffer and supports signed and unsigned integral types, `bool`, character types, `std::string`, `__int128_t`, and `__uint128_t`. Signed integer input accepts an optional `+` or `-`; unsigned input is for non-negative values. Integer input is not range-checked, so each value must fit its destination type. Floating-point input is not provided.

| Operation | Description |
| --- | --- |
| `cin >> value` | Read a supported scalar or string. |
| `cin >> vector` / `cin >> pair` | Read elements recursively when `<slow>` is included after `<fast>`. |
| `if (cin)` | Check whether the most recent extraction succeeded. |

`char`, `signed char`, and `unsigned char` read one non-whitespace character. For a byte interpreted as a number, read an integer type instead.

## Output

The output object buffers writes in 64 KiB and flushes when full or when the program exits. It supports integral types, 128-bit integers, booleans, character types, C strings, `std::string`, and floating-point values. Floating-point values use `snprintf` with `%g` or `%Lg` formatting. Newlines and spaces must be written explicitly.

| Operation | Description |
| --- | --- |
| `cout << value` | Write a supported scalar or string. |
| `cout << vector` / `cout << pair` | Write elements when `<slow>` is included after `<fast>`. |
| `cout.flush()` | Flush buffered output immediately. |

## Complete example with `<slow>`

`<slow>` supplies vector and pair stream operators for both standard streams and `fast_io` streams.

```cpp
#include <bits/stdc++.h>
#include <fast>
#include <slow>

int main() {
    int n;
    if (!(cin >> n)) return 0;

    std::vector<long long> values(n);
    cin >> values;

    long long sum = 0;
    for (long long value : values) sum += value;
    cout << sum << '\n';
}
```

# `sb` Command

`sb` compiles the selected C++ source, runs every matching saved input file,
and compares each result with its expected output.

## Usage

```sh
sb [file.cpp]
```

With no argument, `sb` uses `.cp-tools-current`. The `.cpp` suffix is optional.
It looks for `<source>_test*.in` files and pairs each one with a `.out` file of
the same name. It reports `Accepted`, `Wrong Answer`, `Runtime error`, or
`Compilation error` for each case. A missing `.out` file is reported as a
wrong answer. The command exits with status zero only when all cases pass.

```sh
sb
sb A
```

Compiler and program stderr are suppressed. Set `CXX` to select the compiler,
`CP_INCLUDE` or `CP_LIBRARY` to override the library include path, and
`GPP_ARGS` to add compiler flags.

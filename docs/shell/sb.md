# `sb` Command

## Purpose

`sb` compiles the selected C++ source, runs every matching saved input file,
and compares each result with its expected output.

## Usage and arguments

```sh
sb [file.cpp]
```

With no argument, `sb` uses `.cp-tools-current`. The `.cpp` suffix is optional.
It searches for `<source>_test*.in` files and pairs each with a `.out` file of
the same name.

## Results and exit status

The command reports `Accepted`, `Wrong Answer`, `Runtime error`, or
`Compilation error` for each case. A missing `.out` file is reported as a
wrong answer. `sb` exits with status zero only when every case passes.
Compiler and program stderr are suppressed.

```sh
sb
sb A
```

## Environment

| Variable | Purpose |
| --- | --- |
| `CXX` | Select the C++ compiler. |
| `CP_INCLUDE` / `CP_LIBRARY` | Override the library include path. |
| `GPP_ARGS` | Add compiler flags. |

# `rr` Command

## Purpose

`rr` compiles and runs a C++ or C source file, or runs a Python script. It can
use the source remembered by `mkcpp` or `mkpy`.

## Usage and arguments

```sh
rr [file]
```

With no argument, `rr` uses the source path stored in `.cp-tools-current`. A
file path selects another source. Supported extensions are `.cpp`, `.c`, and
`.py`; if no recognized extension is present, `.cpp` is appended. Only one
source path can be passed, and the source file must exist.

## Build and run behavior

| Source | Behavior |
| --- | --- |
| `.cpp` | Compile as C++17 with `-O2 -Wall -Wextra -DLOCAL` and the project library include path, then run the executable. |
| `.c` | Compile as C17 with `-O2 -Wall -Wextra`, then run the executable. |
| `.py` | Run with `${PYTHON:-python3}`. |

The command displays program stdout and stderr, elapsed time, and exit code.
Its exit code matches the program's exit code. A missing source, unsupported
extension, compiler failure, or missing Python interpreter returns a nonzero
status. Compilation failures are reported as `Compilation error`.

## Input redirection

Standard input is passed to the program, so shell input redirection works for
all supported source types:

```sh
rr A.cpp < A_test01.in
rr A.c < A_test01.in
rr A.py < A_test01.in
```

## Environment

| Variable | Purpose |
| --- | --- |
| `CXX` | Select the C++ compiler. |
| `CC` | Select the C compiler. |
| `CP_INCLUDE` / `CP_LIBRARY` | Override the library include path. |
| `PYTHON` | Select the Python interpreter. |

## Examples

```sh
rr                 # Run the remembered source.
rr A               # Run A.cpp.
rr A.cpp           # Run a C++ source directly.
rr sample.c        # Compile and run C.
rr solution.py     # Run Python.
```

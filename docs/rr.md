# `rr` Command

`rr` compiles and runs a C++ or C source file, or runs a Python script. It
remembers the source created or selected by `mkcpp` or `mkpy`.

## Usage

```sh
rr [file]
```

With no argument, `rr` uses the source path stored in `.cp-tools-current`. Pass
a file path to run a different source. The supported extensions are `.cpp`,
`.c`, and `.py`. If the path has no recognized extension, `.cpp` is appended.
For example, `rr A` selects `A.cpp`.

Only one source path can be passed. The source file must exist.

## Build and run behavior

| Source | Behavior |
| --- | --- |
| `.cpp` | Compiles as C++17 with `-O2 -Wall -Wextra -DLOCAL` and the project library include path, then runs the executable. Set `CXX` to choose another C++ compiler. Set `CP_INCLUDE` or `CP_LIBRARY` to override the include path. |
| `.c` | Compiles as C17 with `-O2 -Wall -Wextra`, then runs the executable. Set `CC` to choose another C compiler. |
| `.py` | Runs the script with `${PYTHON:-python3}`. Set `PYTHON` to choose another Python interpreter. |

Compilation failures are reported as `Compilation error`. The command displays
the program's stdout and stderr, elapsed time, and exit code. Its own exit code
matches the program's exit code; a missing source, unsupported extension,
compiler failure, or missing Python interpreter returns a nonzero status.

## Input redirection

Standard input is passed to the program, so shell input redirection works for
all supported source types:

```sh
rr A.cpp < A_test01.in
rr A.c < A_test01.in
rr A.py < A_test01.in
```

The program output is captured by `rr` and displayed with stdout/stderr labels
and a timing summary.

## Examples

```sh
rr                 # Run the remembered source.
rr A               # Run A.cpp.
rr A.cpp           # Run a C++ source directly.
rr sample.c        # Compile and run C.
rr solution.py     # Run Python.
```

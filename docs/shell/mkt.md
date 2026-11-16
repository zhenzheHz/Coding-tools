# `mkt` Command

`mkt` interactively saves one input/output test pair for the current C++ source.
The files are used by `sb`.

## Usage

```sh
mkt [test-number]
```

Run `mkcpp` first so `.cp-tools-current` identifies the source. Without a test
number, `mkt` finds the next unused number, starting at `01`. A positive number
can be supplied explicitly; if either corresponding file already exists, the
command stops rather than overwriting it.

For source `A.cpp`, the command creates `A_test01.in` and `A_test01.out` (or the
requested number). Enter the input, then press `Ctrl-D`; enter the expected
output and press `Ctrl-D` again.

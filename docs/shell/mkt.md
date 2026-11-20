# `mkt` Command

## Purpose

`mkt` interactively saves one input/output test pair for the current C++ source.
The files are used by `sb`.

## Usage and arguments

```sh
mkt [test-number]
```

Run `mkcpp` first so `.cp-tools-current` identifies the source. Without an
argument, `mkt` chooses the next unused positive number, starting at `01`. A
positive number can be supplied explicitly. If either corresponding file
already exists, the command stops rather than overwriting it.

## Behavior

For source `A.cpp`, the command creates `A_test01.in` and `A_test01.out` (or the
requested number). Enter the input and press `Ctrl-D`, then enter the expected
output and press `Ctrl-D` again.

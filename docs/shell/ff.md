# `ff` Command

## Purpose

`ff` forgets the source recorded by `mkcpp` or `mkpy` in the current directory
and can remove that source's numbered test files.

## Usage

```sh
ff
```

Run the command from the directory whose remembered source you want to clear.
It takes no arguments.

## Behavior

The command displays the remembered source and matching test files before
asking for confirmation. Enter `y`, `Y`, `yes`, or `YES` to delete those test
files and remove `.cp-tools-current`. Any other response cancels the operation.

Only files matching `<source>_test<number>.in` and
`<source>_test<number>.out` are considered. For example, for `A.cpp`,
`A_test01.in` and `A_test01.out` are removed while `A.cpp` is kept.

# `ff` Command

`ff` forgets the source recorded by `mkcpp` or `mkpy` in the current directory.
After confirmation, it deletes matching numbered `.in` and `.out` test files
while keeping the source file.

## Usage

```sh
ff
```

The command shows the remembered source and the matching files before asking
for confirmation. Enter `y`, `Y`, `yes`, or `YES` to delete those test files
and remove `.cp-tools-current`. Any other response cancels the operation.
Run it from the directory whose remembered source you want to clear.

Only files matching `<source>_test<number>.in` and
`<source>_test<number>.out` are considered. For example, for `A.cpp`, files
`A_test01.in` and `A_test01.out` are removed, while `A.cpp` is kept.

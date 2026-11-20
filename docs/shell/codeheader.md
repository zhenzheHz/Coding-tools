# `codeheader` Command

## Purpose

`codeheader` prints the author and date comment used at the top of generated
source files.

## Usage and arguments

```sh
codeheader [prefix] [label]
```

| Argument | Description |
| --- | --- |
| `prefix` | Comment prefix; defaults to `//`. Use `#` for Python. |
| `label` | Optional extra comment text. The special value `c` prints a `Pre-compiled :` timestamp label. |

Examples:

```sh
codeheader
codeheader '#'
codeheader '//' Main
codeheader '//' c
```

## Environment

`CP_AUTHOR` sets the author name. If unset, the command uses `$USER`, then
`unknown` if `$USER` is also unset.

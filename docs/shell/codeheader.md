# `codeheader` Command

`codeheader` prints the author/date comment used at the top of generated
source files.

## Usage

```sh
codeheader [prefix] [label]
```

The default prefix is `//`. Use `#` for a Python comment. `CP_AUTHOR` sets the
author; if unset, the command uses `$USER` and then `unknown`.

```sh
codeheader
codeheader '#'
codeheader '//' Main
codeheader '//' c
```

The second argument `c` prints a `Pre-compiled :` timestamp label. Any other
second argument is printed as an additional comment line.

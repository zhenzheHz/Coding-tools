# `mkcpp` Command

`mkcpp` creates a C++ source file from the shared template and records it as
the current source for other commands.

## Usage

```sh
mkcpp [-t] <filename[.cpp]>
```

The `.cpp` suffix is optional. Existing files are never overwritten. The new
file starts with the author/date header from `codeheader`, followed by
`library/default_code.cpp`, and its path is saved in `.cp-tools-current` in the
current working directory.

By default, the command opens the file using `CODE_EDITOR`, then `EDITOR`, then
`code`. Use `-t` to create the file without opening an editor:

```sh
mkcpp A
mkcpp -t solution.cpp
```

Set `CP_TEMPLATE` to use a different template. `CP_LIBRARY` selects the
default template directory when `CP_TEMPLATE` is unset. `CP_AUTHOR` controls
the generated author name.

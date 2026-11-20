# `mkcpp` Command

## Purpose

`mkcpp` creates a C++ source file from the shared template and records it as
the current source for other commands.

## Usage and arguments

```sh
mkcpp [-t] <filename[.cpp]>
```

The `.cpp` suffix is optional. Existing files are never overwritten. `-t`
creates the file without opening an editor.

## Behavior

The new file starts with the author/date header from `codeheader`, followed by
`library/default_code.cpp`. Its path is saved in `.cp-tools-current` in the
current working directory. By default, the command opens the file using
`CODE_EDITOR`, then `EDITOR`, then `code`.

```sh
mkcpp A
mkcpp -t solution.cpp
```

## Environment

| Variable | Purpose |
| --- | --- |
| `CP_TEMPLATE` | Select a custom source template. |
| `CP_LIBRARY` | Select the default template directory when `CP_TEMPLATE` is unset. |
| `CP_AUTHOR` | Set the author name in the generated header. |
| `CODE_EDITOR` / `EDITOR` | Select the editor, in that order; `code` is the fallback. |

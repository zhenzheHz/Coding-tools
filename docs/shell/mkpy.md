# `mkpy` Command

`mkpy` creates a Python source file with the author/date comment and records it
as the current source for commands such as `rr`.

## Usage

```sh
mkpy [-t] <filename[.py]>
```

The `.py` suffix is optional. Existing files are never overwritten. The new
file contains the header produced by `codeheader '#'`, and its path is saved
in `.cp-tools-current` in the current working directory.

By default, the command opens the file using `CODE_EDITOR`, then `EDITOR`, then
`code`. Use `-t` to create it without opening an editor:

```sh
mkpy answer
mkpy -t script.py
```

Set `CP_AUTHOR` to change the author name in the generated header.

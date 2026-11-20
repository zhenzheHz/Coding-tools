# `mkpy` Command

## Purpose

`mkpy` creates a Python source file with an author/date comment and records it
as the current source for commands such as `rr`.

## Usage and arguments

```sh
mkpy [-t] <filename[.py]>
```

The `.py` suffix is optional. Existing files are never overwritten. Use `-t`
to create the file without opening an editor.

## Behavior

The file contains the header produced by `codeheader '#'`. Its path is saved in
`.cp-tools-current` in the current working directory. By default, the command
opens the file using `CODE_EDITOR`, then `EDITOR`, then `code`.

```sh
mkpy answer
mkpy -t script.py
```

## Environment

| Variable | Purpose |
| --- | --- |
| `CP_AUTHOR` | Set the author name in the generated header. |
| `CODE_EDITOR` / `EDITOR` | Select the editor, in that order; `code` is the fallback. |

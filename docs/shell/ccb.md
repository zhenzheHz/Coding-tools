# `ccb` Command

## Purpose

`ccb` preprocesses one C++ source file into a single source suitable for an
online judge. With no file argument, it uses the source remembered by `mkcpp`.

## Usage and arguments

```sh
ccb [file.cpp]
```

The `.cpp` suffix is optional and is appended when omitted. Pass only one
existing source path.

## Behavior

When run in an interactive terminal, `ccb` copies the prepared source to the
first available clipboard utility: `pbcopy`, `wl-copy`, `xclip`, `xsel`, or
`clip.exe`. If none is available, it prints the result. When stdout is
redirected or piped, it always writes the prepared source to stdout:

```sh
ccb A.cpp > A_precompiled.cpp
```

The generated file includes source/compiler metadata and an original-source
comment at the end. `bits/stdc++.h` and `atcoder/all` are restored as top-level
includes after preprocessing.

## Compiler and environment

The command uses `${CXX:-g++-16}` with `ONLINE_JUDGE` defined. It searches the
source directory and library include directory for headers.

| Variable | Purpose |
| --- | --- |
| `CXX` | Select the C++ compiler. |
| `CP_LIBRARY` / `CP_INCLUDE` | Override the library include path. |
| `CCB_AUTHOR` / `CP_AUTHOR` | Set the author name in generated metadata. |

# `ccb` Command

`ccb` preprocesses a C++ source file into a single source suitable for an
online judge. It uses the same file remembered by `mkcpp` when no path is
given.

## Usage

```sh
ccb [file.cpp]
```

If the file name has no `.cpp` suffix, `ccb` appends it. Only one path is
accepted. The source must exist.

## Output

When run in an interactive terminal, `ccb` copies the prepared source to the
first available clipboard utility (`pbcopy`, `wl-copy`, `xclip`, `xsel`, or
`clip.exe`). If no clipboard utility is available, it prints the result.
When stdout is redirected or piped, it always writes the prepared source to
stdout, so it can be saved directly:

```sh
ccb A.cpp > A_precompiled.cpp
```

The generated file includes source/compiler metadata and an original-source
comment at the end. `bits/stdc++.h` and `atcoder/all` are restored as top-level
includes after preprocessing.

## Compiler and environment

The command uses `${CXX:-g++-16}` with `ONLINE_JUDGE` defined. It searches the
source directory and library include directory for headers. Set `CP_LIBRARY`
or `CP_INCLUDE` to override the library path. Set `CCB_AUTHOR` or `CP_AUTHOR`
to change the author name in the generated metadata.

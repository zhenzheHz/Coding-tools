<div align="center">
  <h1>Coding Tools</h1>
  <p>A small Bash toolkit for competitive programming</p>
  <p>
    <img alt="Bash" src="https://img.shields.io/badge/Shell-Bash-4EAA25?logo=gnubash&logoColor=white">
    <img alt="C++17" src="https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white">
  </p>
  <p><a href="README_TW.md">繁體中文</a> | English</p>
</div>

---

Coding Tools provides short terminal commands for the everyday competitive programming workflow: creating a source file, compiling and running it, preparing judge-ready code, saving test cases, and checking results.

## Features

- Create a C++ source file from a shared template and remember the current source.
- Compile and run C++ or C programs with a local development define.
- Preprocess C++ source for online judges and copy it to the clipboard when available.
- Save numbered input/output pairs and compare them with the program's output.

## Installation

### Requirements

- Bash and a C++ compiler with `bits/stdc++.h` support.
- macOS: `g++-16` available in `PATH` (for example, from Homebrew GCC).
- Linux: `g++-16` or `g++` available in `PATH`.
- Windows: Git Bash and MinGW GCC, either bundled with Code::Blocks or available in `PATH`. PowerShell and Command Prompt are not supported.

The installer also detects a C compiler for the optional `.c` support in `rr`.

Windows Git Bash support is implemented but has not yet been verified on a Windows host.

### Install

```sh
git clone https://github.com/zhenzheHz/Coding-tools.git
cd Coding-tools
bash install.sh
```

The installer asks for the author name used in generated source headers. It then adds the command directory and project paths to the Bash or Zsh startup file. Restart the terminal, or source the startup file named by the installer.

## Commands

| Command | Usage | Description |
| --- | --- | --- |
| `mkcpp` | `mkcpp <filename[.cpp]>` | Creates a source file from `library/default_code.cpp`, adds the author/date header, records it as the current file, and opens it when an editor is available. |
| `rr` | `rr [file.cpp]` | Compiles and runs the current or specified source. C++ uses C++17, `-O2`, `-Wall`, `-Wextra`, `-DLOCAL`, and the project library include path. Input redirection is supported. It also accepts `.c` files. |
| `ccb` | `ccb [file.cpp]` | Preprocesses source for an online judge. It copies the result to the clipboard when a clipboard utility is available. Redirect stdout to save it to a file. |
| `mkt` | `mkt [test-number]` | Interactively saves input and expected output as numbered test files. Press `Ctrl-D` to finish each entry. |
| `sb` | `sb [file.cpp]` | Runs every matching test pair, compares output, and reports one result per test. Program and compiler stderr are suppressed. |
| `codeheader` | `codeheader` | Prints the standard author/date header used by `mkcpp`. |

### Examples

```sh
mkcpp A
rr
rr A.cpp < A_test01.in
mkt
sb
ccb A.cpp > A_precompiled.cpp
```

`mkt` creates `A_test01.in` and `A_test01.out`, then increments the number for subsequent pairs. `sb` displays results such as `01 > Accepted.`.

## Documentation

| Path | Contents |
| --- | --- |
| [`shell/`](shell/) | Bash command implementations. |
| [`library/`](library/) | Reusable C++ headers and snippets. |
| [`library/seg_tree/`](library/seg_tree/) | Segment tree implementations. |
| [`library/default_code.cpp`](library/default_code.cpp) | Starter source copied by `mkcpp`. |
| [`install.sh`](install.sh) | Environment and command-path setup. |

See [README_TW.md](README_TW.md) for Traditional Chinese.

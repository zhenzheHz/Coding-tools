<div align="center">
  <h1>Coding Tools</h1>
  <p>A small Bash toolkit for competitive programming</p>
  <p>
    <img alt="Bash" src="https://img.shields.io/badge/Shell-Bash-4EAA25?logo=gnubash&logoColor=white">
    <img alt="C++17" src="https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white">
  </p>
  <p><a href="README_TW.md">繁體中文</a> | <a href="README.md">English</a></p>
</div>

---

Coding Tools provides short terminal commands for the everyday competitive programming workflow: creating a source file, compiling and running it, preparing judge-ready code, saving test cases, and checking results.

## Features

- Create a C++ source file from a shared template and remember the current source.
- Compile and run C++ or C programs with a local development define.
- Preprocess C++ source for online judges and copy it to the clipboard when available.
- Save numbered input/output pairs and compare them with the program's output.

## Installation

### macOS

Install Homebrew GCC and make sure `g++-16` is available in `PATH`:

```sh
brew install gcc
command -v g++-16
```

Then clone the project and run the installer:

```sh
git clone https://github.com/zhenzheHz/Coding-tools.git
cd Coding-tools
bash install.sh
```

### Linux

Install GCC with your distribution's package manager. The installer prefers `g++-16` and falls back to `g++`; the compiler must support `bits/stdc++.h`.

For Debian or Ubuntu:

```sh
sudo apt update
sudo apt install g++
```

Then clone the project and run the installer:

```sh
git clone https://github.com/zhenzheHz/Coding-tools.git
cd Coding-tools
bash install.sh
```

### Windows (Git Bash)

Install Git for Windows and a MinGW GCC toolchain. The installer can find MinGW bundled with Code::Blocks in common installation locations, or use MinGW available in Git Bash's `PATH`. PowerShell and Command Prompt are not supported.

Open Git Bash, then clone the project and run the installer:

```sh
git clone https://github.com/zhenzheHz/Coding-tools.git
cd Coding-tools
bash install.sh
```

Windows Git Bash support is implemented but has not yet been verified on a Windows host.

On all platforms, the installer asks for the author name used in generated source headers and adds the command directory and project paths to the shell startup file. Restart the terminal, or source the startup file named by the installer. It also detects a C compiler for the optional `.c` support in `rr`.

## Commands

| Command | Usage | Description |
| --- | --- | --- |
| `mkcpp` | `mkcpp [-t] <filename[.cpp]>` | Creates a source file from `library/default_code.cpp`, adds the author/date header, and records it as the current file. Use `-t` to skip opening an editor. |
| `rr` | `rr [file.cpp]` | Compiles and runs the current or specified source. C++ uses C++17, `-O2`, `-Wall`, `-Wextra`, `-DLOCAL`, and the project library include path. Input redirection is supported. It also accepts `.c` files. |
| `ccb` | `ccb [file.cpp]` | Preprocesses source for an online judge. It copies the result to the clipboard when a clipboard utility is available. Redirect stdout to save it to a file. |
| `mkt` | `mkt [test-number]` | Interactively saves input and expected output as numbered test files. Press `Ctrl-D` to finish each entry. |
| `sb` | `sb [file.cpp]` | Runs every matching test pair, compares output, and reports one result per test. Program and compiler stderr are suppressed. |
| `ff` | `ff` | Forgets the remembered source and, after confirmation, deletes its matching test files while keeping the source. |
| `codeheader` | `codeheader` | Prints the standard author/date header used by `mkcpp`. |

### Examples

```sh
mkcpp A
mkcpp -t B
rr
rr A.cpp < A_test01.in
mkt
sb
ccb A.cpp > A_precompiled.cpp
```

`mkt` creates `A_test01.in` and `A_test01.out`, then increments the number for subsequent pairs. `sb` displays results such as `01 > Accepted.`.

## Documentation

Library documentation is available in Markdown:

- [Disjoint Set Union](docs/disjoint_set.md)
- [Fenwick Tree](docs/fenwick_tree.md)
- [Ordered Set](docs/ordered_set.md)
- [Sparse Table](docs/sparse_table.md)
- [Segment Trees: `magic_tree` and `seg_tree`](docs/segment_tree.md)

Printable PDF references may be added under [`docs/`](docs/) separately.

See [README_TW.md](README_TW.md) for Traditional Chinese.

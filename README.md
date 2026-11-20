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

- Create C++ or Python source files and remember the current source.
- Compile and run C or C++ programs, or run Python scripts.
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

### Windows (PowerShell and Git Bash)

Install [Git for Windows](https://git-scm.com/download/win) and a MinGW GCC toolchain. The installer can find MinGW bundled with Code::Blocks in common installation locations, or use MinGW available in `PATH`.

Clone the project and run the native PowerShell installer:

```powershell
git clone https://github.com/zhenzheHz/Coding-tools.git
cd Coding-tools
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\install.ps1
```

After installation, open Git Bash to use the commands. The installer is PowerShell-based, but the commands themselves are Bash scripts and still require Git Bash.

The installer asks for the author name used in generated source headers and adds the command directory and project paths to the shell startup file. Restart Git Bash, or source the startup file named by the installer. It also detects a C compiler for the optional `.c` support in `rr`.

## Commands

| Command | Usage | Description |
| --- | --- | --- |
| `mkcpp` | `mkcpp [-t] <filename[.cpp]>` | Creates a source file from `library/default_code.cpp`, adds the author/date header, and records it as the current file. Use `-t` to skip opening an editor. |
| `mkpy` | `mkpy [-t] <filename[.py]>` | Creates a Python source containing only the author/date code header and records it as the current file. Use `-t` to skip opening an editor. |
| `rr` | `rr [file]` | Compiles and runs the remembered source, or the specified `.cpp`, `.c`, or `.py` file. A name without an extension is treated as `.cpp`. C++ uses C++17, `-O2`, `-Wall`, `-Wextra`, `-DLOCAL`, and the project library include path; C uses C17; Python uses `${PYTHON:-python3}`. Input redirection is supported. See [the `rr` guide](docs/shell/rr.md). |
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

### Shell

| Command | Description | Guide |
| --- | --- | --- |
| `ccb` | Preprocess C++ source for an online judge. | [Usage](docs/shell/ccb.md) |
| `codeheader` | Print the author/date source header. | [Usage](docs/shell/codeheader.md) |
| `ff` | Forget the current source and optionally remove its test files. | [Usage](docs/shell/ff.md) |
| `mkcpp` | Create a C++ source file from the shared template. | [Usage](docs/shell/mkcpp.md) |
| `mkpy` | Create a Python source file with the author/date header. | [Usage](docs/shell/mkpy.md) |
| `mkt` | Save an input/output test pair. | [Usage](docs/shell/mkt.md) |
| `rr` | Compile and run C/C++ or run Python. | [Usage](docs/shell/rr.md) |
| `sb` | Run saved tests and compare expected output. | [Usage](docs/shell/sb.md) |

### Data Structures

| Structure | Header | Guide |
| --- | --- | --- |
| Disjoint Set Union | `<disjoint_set>` | [Documentation](docs/disjoint_set.md) |
| Fenwick Tree | `<fenwick>` | [Documentation](docs/fenwick_tree.md) |
| Hash Map | `<hash_map>` | [Documentation](docs/hash_map.md) |
| Heap | `<heap>` | [Documentation](docs/heap.md) |
| Monotonic Stack Queries | `<monotonic_stack>` | [Documentation](docs/monotonic_stack.md) |
| Ordered Set | `<ordered_set>` | [Documentation](docs/ordered_set.md) |
| Segment Trees | `<magic_tree>` / `<seg_tree>` | [Documentation](docs/segment_tree.md) |
| Sparse Table | `<sparse_table>` | [Documentation](docs/sparse_table.md) |
| Treap | `<treap>` | [Documentation](docs/treap.md) |
| Trie | `<trie>` | [Documentation](docs/trie.md) |
| van Emde Boas Tree | `<veb_tree>` | [Documentation](docs/van_emde_boas.md) |

### Math

| Topic | Header | Guide |
| --- | --- | --- |
| 128-bit Integers | `<int128>` | [Documentation](docs/int128.md) |

### Graph

Graph library documentation will be listed here as it is added.

Printable PDF references may be added under [`docs/`](docs/) separately.

See [README_TW.md](README_TW.md) for Traditional Chinese.

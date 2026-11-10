<div align="center">
  <h1>Coding Tools</h1>
  <p>為競程日常流程準備的 Bash 指令工具</p>
  <p>
    <img alt="Bash" src="https://img.shields.io/badge/Shell-Bash-4EAA25?logo=gnubash&logoColor=white">
    <img alt="C++17" src="https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white">
  </p>
  <p>繁體中文 | <a href="README.md">English</a></p>
</div>

---

Coding Tools 提供一組簡短的終端機指令，協助處理競程常見流程：建立程式檔、編譯執行、準備提交到 judge 的程式碼、儲存測資，以及比對執行結果。

## 功能

- 從共用範本建立 C++ 程式檔，並記錄目前使用的檔案。
- 使用本機開發用 define 編譯和執行 C++ 或 C 程式。
- 預處理 C++ 程式碼；系統有剪貼簿工具時可直接複製結果。
- 建立編號 input/output 測資，並比對程式輸出。

## Installation

### 系統需求

- Bash，以及支援 `bits/stdc++.h` 的 C++ 編譯器。
- macOS：`PATH` 中需有 `g++-16`（例如由 Homebrew GCC 提供）。
- Linux：`PATH` 中需有 `g++-16` 或 `g++`。
- Windows：使用 Git Bash，並安裝 MinGW GCC；可使用 Code::Blocks 內附版本或已加入 `PATH` 的 MinGW。PowerShell 和命令提示字元不支援。

`rr` 也支援 `.c` 檔；安裝程式會偵測可用的 C 編譯器。

Windows Git Bash 支援已加入，但尚未在 Windows 主機上驗證。

### 安裝方式

```sh
git clone https://github.com/zhenzheHz/Coding-tools.git
cd Coding-tools
bash install.sh
```

安裝程式會詢問要寫入程式標頭的作者姓名，並將指令路徑與專案路徑加入 Bash 或 Zsh 的啟動設定檔。完成後請重開終端機，或載入安裝程式提示的設定檔。

## 指令

| 指令 | 用法 | 說明 |
| --- | --- | --- |
| `mkcpp` | `mkcpp <filename[.cpp]>` | 根據 `library/default_code.cpp` 建立程式檔、加入作者與日期標頭、記錄目前檔案，並在有可用編輯器時開啟。 |
| `rr` | `rr [file.cpp]` | 編譯並執行目前或指定的程式檔。C++ 使用 C++17、`-O2`、`-Wall`、`-Wextra`、`-DLOCAL` 和專案 library 路徑；支援輸入重導向，也接受 `.c` 檔。 |
| `ccb` | `ccb [file.cpp]` | 預處理程式碼供線上 judge 使用。有剪貼簿工具時會複製結果，也可以用 stdout 重導向存成檔案。 |
| `mkt` | `mkt [test-number]` | 互動式儲存 input 和預期 output。每段輸入完成後按 `Ctrl-D`。 |
| `sb` | `sb [file.cpp]` | 執行所有相符的測資並比對輸出；每筆測資各自顯示結果。程式和編譯器的 stderr 會被丟棄。 |
| `codeheader` | `codeheader` | 輸出 `mkcpp` 使用的作者與日期標頭。 |

### 範例

```sh
mkcpp A
rr
rr A.cpp < A_test01.in
mkt
sb
ccb A.cpp > A_precompiled.cpp
```

`mkt` 會建立 `A_test01.in` 和 `A_test01.out`，之後自動遞增編號。`sb` 會顯示如 `01 > Accepted.` 的結果。

## Documentation

| 路徑 | 內容 |
| --- | --- |
| [`shell/`](shell/) | Bash 指令實作。 |
| [`library/`](library/) | 可重複使用的 C++ 標頭與程式片段。 |
| [`library/seg_tree/`](library/seg_tree/) | 線段樹實作。 |
| [`library/default_code.cpp`](library/default_code.cpp) | `mkcpp` 使用的程式範本。 |
| [`install.sh`](install.sh) | 環境與指令路徑設定。 |

英文版請參考 [README.md](README.md)。

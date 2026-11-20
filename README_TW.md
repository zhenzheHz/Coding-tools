<div align="center">
  <h1>Coding Tools</h1>
  <p>為競程日常流程準備的 Bash 指令工具</p>
  <p>
    <img alt="Bash" src="https://img.shields.io/badge/Shell-Bash-4EAA25?logo=gnubash&logoColor=white">
    <img alt="C++17" src="https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white">
  </p>
  <p><a href="README_TW.md">繁體中文</a> | <a href="README.md">English</a></p>
</div>

---

Coding Tools 提供一組簡短的終端機指令，協助處理競程常見流程：建立程式檔、編譯執行、準備提交到 judge 的程式碼、儲存測資，以及比對執行結果。

## 功能

- 建立 C++ 或 Python 程式檔，並記錄目前使用的檔案。
- 編譯和執行 C++、C 程式，或執行 Python 程式。
- 預處理 C++ 程式碼；系統有剪貼簿工具時可直接複製結果。
- 建立編號 input/output 測資，並比對程式輸出。

## 安裝 Installation

### macOS

安裝 Homebrew GCC，並確認 `PATH` 中可以找到 `g++-16`：

```sh
brew install gcc
command -v g++-16
```

接著 clone 專案並執行安裝程式：

```sh
git clone https://github.com/zhenzheHz/Coding-tools.git
cd Coding-tools
bash install.sh
```

### Linux

使用發行版的套件管理工具安裝 GCC。安裝程式會優先使用 `g++-16`，若找不到則使用 `g++`；編譯器需支援 `bits/stdc++.h`。

Debian 或 Ubuntu 可使用：

```sh
sudo apt update
sudo apt install g++
```

接著 clone 專案並執行安裝程式：

```sh
git clone https://github.com/zhenzheHz/Coding-tools.git
cd Coding-tools
bash install.sh
```

### Windows（PowerShell 與 Git Bash）

請先安裝 [Git for Windows](https://git-scm.com/download/win) 和 MinGW GCC。安裝程式會搜尋常見位置的 Code::Blocks 內附 MinGW，也可使用已加入 `PATH` 的 MinGW。

clone 專案後，在 PowerShell 執行原生安裝程式：

```powershell
git clone https://github.com/zhenzheHz/Coding-tools.git
cd Coding-tools
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\install.ps1
```

安裝完成後，請開啟 Git Bash 使用指令。安裝程式本身使用 PowerShell，但指令仍是 Bash 腳本，因此執行時需要 Git Bash。

安裝程式會詢問要寫入程式標頭的作者姓名，並將指令路徑與專案路徑加入 shell 啟動設定檔。完成後請重開 Git Bash，或載入安裝程式提示的設定檔。安裝程式也會偵測 C 編譯器，供 `rr` 選擇性編譯 `.c` 檔使用。

## 指令

| 指令 | 用法 | 說明 |
| --- | --- | --- |
| `mkcpp` | `mkcpp [-t] <filename[.cpp]>` | 根據 `library/default_code.cpp` 建立程式檔、加入作者與日期標頭並記錄目前檔案；加上 `-t` 時不自動開啟編輯器。 |
| `mkpy` | `mkpy [-t] <filename[.py]>` | 建立只含作者與日期 code header 的 Python 檔並記錄目前檔案；加上 `-t` 時不自動開啟編輯器。 |
| `rr` | `rr [file]` | 編譯或執行目前記錄的程式檔，或指定的 `.cpp`、`.c`、`.py` 檔；未附副檔名時視為 `.cpp`。C++ 使用 C++17、`-O2`、`-Wall`、`-Wextra`、`-DLOCAL` 和專案 library 路徑；C 使用 C17；Python 使用 `${PYTHON:-python3}`。支援輸入重導向。詳見 [`rr` 使用說明](docs/shell/rr.md)。 |
| `ccb` | `ccb [file.cpp]` | 預處理程式碼供線上 judge 使用。有剪貼簿工具時會複製結果，也可以用 stdout 重導向存成檔案。 |
| `mkt` | `mkt [test-number]` | 互動式儲存 input 和預期 output。每段輸入完成後按 `Ctrl-D`。 |
| `sb` | `sb [file.cpp]` | 執行所有相符的測資並比對輸出；每筆測資各自顯示結果。程式和編譯器的 stderr 會被丟棄。 |
| `ff` | `ff` | 忘記目前記錄的程式檔；確認後刪除該檔名對應的測資檔，保留程式檔。 |
| `codeheader` | `codeheader` | 輸出 `mkcpp` 使用的作者與日期標頭。 |

### 範例

```sh
mkcpp A
mkcpp -t B
rr
rr A.cpp < A_test01.in
mkt
sb
ccb A.cpp > A_precompiled.cpp
```

`mkt` 會建立 `A_test01.in` 和 `A_test01.out`，之後自動遞增編號。`sb` 會顯示如 `01 > Accepted.` 的結果。

## 文件 Documentation

### Shell 指令

| 指令 | 說明 | 文件 |
| --- | --- | --- |
| `ccb` | 預處理 C++ 程式碼供線上 judge 使用。 | [使用說明](docs/shell/ccb.md) |
| `codeheader` | 輸出作者與日期程式標頭。 | [使用說明](docs/shell/codeheader.md) |
| `ff` | 忘記目前程式檔，並可刪除對應測資。 | [使用說明](docs/shell/ff.md) |
| `mkcpp` | 根據共用範本建立 C++ 程式檔。 | [使用說明](docs/shell/mkcpp.md) |
| `mkpy` | 建立含作者與日期標頭的 Python 檔。 | [使用說明](docs/shell/mkpy.md) |
| `mkt` | 儲存一組 input/output 測資。 | [使用說明](docs/shell/mkt.md) |
| `rr` | 編譯並執行 C/C++，或執行 Python。 | [使用說明](docs/shell/rr.md) |
| `sb` | 執行已儲存測資並比對預期輸出。 | [使用說明](docs/shell/sb.md) |

### 資料結構

| 結構 | 標頭 | 文件 |
| --- | --- | --- |
| Disjoint Set Union | `<disjoint_set>` | [說明](docs/disjoint_set.md) |
| Fenwick Tree | `<fenwick>` | [說明](docs/fenwick_tree.md) |
| Hash Map | `<hash_map>` | [說明](docs/hash_map.md) |
| Heap | `<heap>` | [說明](docs/heap.md) |
| Monotonic Stack Queries | `<monotonic_stack>` | [說明](docs/monotonic_stack.md) |
| Ordered Set | `<ordered_set>` | [說明](docs/ordered_set.md) |
| Segment Trees | `<magic_tree>` / `<seg_tree>` | [說明](docs/segment_tree.md) |
| Sparse Table | `<sparse_table>` | [說明](docs/sparse_table.md) |
| Treap | `<treap>` | [說明](docs/treap.md) |
| Trie | `<trie>` | [說明](docs/trie.md) |
| van Emde Boas Tree | `<veb_tree>` | [說明](docs/van_emde_boas.md) |

### 數學

| 主題 | 標頭 | 文件 |
| --- | --- | --- |
| 128-bit 整數 | `<int128>` | [說明](docs/int128.md) |

### 圖論

之後新增的圖論 library 文件會列在這裡。

適合列印的 PDF 參考資料之後可能會另外放在 [`docs/`](docs/) 目錄。

英文版請參考 [README.md](README.md)。

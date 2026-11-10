#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
SHELL_DIR="$ROOT/shell"
LIBRARY_DIR="$ROOT/library"
OS_NAME=$(uname -s)
case "$OS_NAME" in
    Darwin) PLATFORM=macOS; DEFAULT_CXX=g++-16; DEFAULT_CC=gcc-16 ;;
    Linux)
        PLATFORM=Linux
        if command -v g++-16 >/dev/null 2>&1; then DEFAULT_CXX=g++-16; else DEFAULT_CXX=g++; fi
        if command -v gcc-16 >/dev/null 2>&1; then DEFAULT_CC=gcc-16; else DEFAULT_CC=gcc; fi
        ;;
    *) echo "> Unsupported operating system: $OS_NAME (macOS and Linux are supported)." >&2; exit 1 ;;
esac
case "${SHELL##*/}" in
    zsh) RC_FILE="$HOME/.zshrc" ;;
    bash)
        if [[ $PLATFORM == macOS ]]; then RC_FILE="$HOME/.bash_profile"; else RC_FILE="$HOME/.bashrc"; fi
        ;;
    *) echo "> Unsupported shell: ${SHELL:-unknown} (use Bash or Zsh)." >&2; exit 1 ;;
esac
if ! command -v "$DEFAULT_CXX" >/dev/null 2>&1; then
    echo "> C++ compiler not found: $DEFAULT_CXX" >&2
    exit 1
fi
printf '> Enter your name : '
IFS= read -r AUTHOR
if [[ -z "$AUTHOR" ]]; then
    echo '> Name cannot be empty.' >&2
    exit 1
fi
for command_file in codeheader mkcpp rr ccb mkt sb; do
    [[ -f "$SHELL_DIR/$command_file" ]] || { echo "Missing command: $SHELL_DIR/$command_file" >&2; exit 1; }
    chmod +x "$SHELL_DIR/$command_file"
done
mkdir -p "$LIBRARY_DIR"
begin='# >>> competitive-programming tools >>>'
end='# <<< competitive-programming tools <<<'
tmp=$(mktemp)
trap 'rm -f "$tmp"' EXIT
if [[ -f "$RC_FILE" ]]; then awk -v begin="$begin" -v end="$end" '$0 == begin { skip=1; next } $0 == end { skip=0; next } !skip { print }' "$RC_FILE" > "$tmp"; fi
{
    cat "$tmp"
    printf '\n%s\n' "$begin"
    printf 'export CP_TOOLS_ROOT=%q\n' "$ROOT"
    printf 'export CP_LIBRARY=%q\n' "$LIBRARY_DIR"
    printf 'export CP_INCLUDE=%q\n' "$LIBRARY_DIR"
    printf 'export CP_TEMPLATE=%q\n' "$LIBRARY_DIR/default_code.cpp"
    printf 'export CP_AUTHOR=%q\n' "$AUTHOR"
    printf 'export CXX=%q\n' "$DEFAULT_CXX"
    printf 'export CC=%q\n' "$DEFAULT_CC"
    printf 'export PATH=%q:"$PATH"\n' "$SHELL_DIR"
    printf '%s\n' "$end"
} > "$RC_FILE"
echo "> Configured $PLATFORM commands in $RC_FILE"
echo '> Restart your shell or source the rc file to use the commands.'

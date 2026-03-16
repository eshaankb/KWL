#!/usr/bin/env bash
# install.sh — KWL Interpreter installer
# Usage: sudo ./install.sh [/path/to/kwlint]
# If no path is given, looks for ./kwlint next to this script.

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BINARY="$SCRIPT_DIR/kwlint"
INSTALL_DIR="/usr/local/bin"
LIB_DIR="/usr/local/lib/kwl/standard/"
MODULES_SRC="$SCRIPT_DIR/standard/"


if [[ ! -x "$BINARY" ]]; then
    chmod +x "$BINARY"
fi

if [[ $EUID -ne 0 ]]; then
    echo "Error: this script must be run as root (use sudo)."
    exit 1
fi

# ── Install binary ─────────────────────────────────────────────────────────────
mkdir -p "$INSTALL_DIR"
install -m 755 "$BINARY" "$INSTALL_DIR/kwlint"
echo "Installed: $INSTALL_DIR/kwlint"

# ── Install standard modules ───────────────────────────────────────────────────
if [[ -d "$MODULES_SRC" ]]; then
    mkdir -p "$LIB_DIR"
    install -m 644 "$MODULES_SRC"/*.kwl "$LIB_DIR/"
    echo "Installed modules to: $LIB_DIR/"
    ls "$LIB_DIR/"
else
    echo "Note: no standard/ folder found next to install.sh — skipping module install."
    echo "      (loginout is built-in and always works; only needed for mathtools etc.)"
fi

# ── PATH check ────────────────────────────────────────────────────────────────
if ! echo "$PATH" | tr ':' '\n' | grep -qx "$INSTALL_DIR"; then
    echo ""
    echo "Note: $INSTALL_DIR is not in your PATH."
    echo "Add this line to your ~/.bashrc or ~/.zshrc:"
    echo "  export PATH=\"$INSTALL_DIR:\$PATH\""
fi
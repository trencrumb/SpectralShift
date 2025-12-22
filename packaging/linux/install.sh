#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Install SpectralShift plugins and standalone on Linux (tested on Ubuntu 24.04).

Usage: ./install.sh [options]
Options:
  --system           Install to system locations (/usr/lib/vst3, /usr/lib/clap, /usr/local/bin).
  --user             Install to user locations (~/.vst3, ~/.clap, ~/.local/bin). This is the default.
  --bin-dir DIR      Override the standalone install dir (default depends on mode).
  --dry-run          Print the actions without performing them.
  -y, --yes          Replace existing files without prompting.
  -h, --help         Show this message.
Environment overrides:
  VST3_DIR, CLAP_DIR, BIN_DIR can override user-mode destinations.
EOF
}

SYSTEM=false
DRY_RUN=false
ASSUME_YES=false
BIN_OVERRIDE=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --system) SYSTEM=true ;;
    --user) SYSTEM=false ;;
    --bin-dir)
      if [[ $# -lt 2 ]]; then
        echo "--bin-dir requires a path" >&2
        exit 1
      fi
      BIN_OVERRIDE="$2"
      shift
      ;;
    --dry-run) DRY_RUN=true ;;
    -y|--yes) ASSUME_YES=true ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      usage
      exit 1
      ;;
  esac
  shift
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

VST3_SRC="$SCRIPT_DIR/VST3/Spectral Shift.vst3"
LV2_SRC="$SCRIPT_DIR/LV2/Spectral Shift.lv2"
CLAP_SRC="$SCRIPT_DIR/CLAP/Spectral Shift.clap"
APP_SRC="$SCRIPT_DIR/Standalone/Spectral Shift"

for src in "$VST3_SRC" "$LV2_SRC" "$CLAP_SRC" "$APP_SRC"; do
  if [[ ! -e "$src" ]]; then
    echo "Missing artifact: $src" >&2
    exit 1
  fi
done

if "$SYSTEM"; then
  VST3_DEST="/usr/lib/vst3"
  LV2_DEST="/usr/lib/lv2"
  CLAP_DEST="/usr/lib/clap"
  BIN_DEST="${BIN_OVERRIDE:-/usr/local/bin}"
else
  VST3_DEST="${VST3_DIR:-$HOME/.vst3}"
  LV2_DEST="${LV2_DIR:-$HOME/.lv2}"
  CLAP_DEST="${CLAP_DIR:-$HOME/.clap}"
  BIN_DEST="${BIN_OVERRIDE:-${BIN_DIR:-$HOME/.local/bin}}"
fi

SUDO=""
if "$SYSTEM" && [[ $EUID -ne 0 ]]; then
  SUDO="sudo"
fi

run() {
  if "$DRY_RUN"; then
    echo "[dry-run]" "$@"
  else
    "$@"
  fi
}

ensure_dir() {
  run ${SUDO:+$SUDO }mkdir -p "$1"
}

confirm_replace() {
  local target="$1"
  if [[ -e "$target" && "$ASSUME_YES" != true ]]; then
    read -r -p "Replace $target? [y/N] " reply
    [[ $reply =~ ^[Yy]$ ]] || return 1
  fi
  return 0
}

install_bundle() {
  local src="$1"
  local dest_dir="$2"
  local name
  name="$(basename "$src")"

  ensure_dir "$dest_dir"
  if confirm_replace "$dest_dir/$name"; then
    run ${SUDO:+$SUDO }rm -rf "$dest_dir/$name"
    run ${SUDO:+$SUDO }cp -a "$src" "$dest_dir/"
    echo "Installed $name to $dest_dir"
  else
    echo "Skipped $name"
  fi
}

install_file() {
  local src="$1"
  local dest_dir="$2"
  local mode="$3"
  local name
  name="$(basename "$src")"

  ensure_dir "$dest_dir"
  if confirm_replace "$dest_dir/$name"; then
    run ${SUDO:+$SUDO }install -m "$mode" "$src" "$dest_dir/$name"
    echo "Installed $name to $dest_dir"
  else
    echo "Skipped $name"
  fi
}

install_bundle "$VST3_SRC" "$VST3_DEST"
install_bundle "$LV2_SRC" "$LV2_DEST"
install_file "$CLAP_SRC" "$CLAP_DEST" 755
install_file "$APP_SRC" "$BIN_DEST" 755

echo "Done."
if ! "$SYSTEM"; then
  echo "VST3 -> $VST3_DEST"
  echo "LV2 -> $LV2_DEST"
  echo "CLAP -> $CLAP_DEST"
  echo "Standalone -> $BIN_DEST"
fi

#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Uninstall SpectralShift plugins and standalone from Linux.

Usage: ./uninstall.sh [options]
Options:
  --system           Uninstall from system locations (/usr/lib/vst3, /usr/lib/clap, /usr/local/bin).
  --user             Uninstall from user locations (~/.vst3, ~/.clap, ~/.local/bin). This is the default.
  --bin-dir DIR      Override the standalone uninstall dir (default depends on mode).
  --dry-run          Print the actions without performing them.
  -y, --yes          Remove files without prompting.
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

VST3_TARGET="$VST3_DEST/Spectral Shift.vst3"
LV2_TARGET="$LV2_DEST/Spectral Shift.lv2"
CLAP_TARGET="$CLAP_DEST/Spectral Shift.clap"
APP_TARGET="$BIN_DEST/Spectral Shift"

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

confirm_remove() {
  local target="$1"
  if [[ "$ASSUME_YES" != true ]]; then
    read -r -p "Remove $target? [y/N] " reply
    [[ $reply =~ ^[Yy]$ ]] || return 1
  fi
  return 0
}

remove_if_exists() {
  local target="$1"
  local name
  name="$(basename "$target")"

  if [[ ! -e "$target" ]]; then
    echo "$name not found at $target (already uninstalled)"
    return 0
  fi

  if confirm_remove "$target"; then
    run ${SUDO:+$SUDO }rm -rf "$target"
    echo "Removed $name from $(dirname "$target")"
  else
    echo "Skipped $name"
  fi
}

echo "Uninstalling SpectralShift..."
echo ""

remove_if_exists "$VST3_TARGET"
remove_if_exists "$CLAP_TARGET"
remove_if_exists "$LV2_TARGET"
remove_if_exists "$APP_TARGET"

echo ""
echo "Uninstall complete."
if ! "$SYSTEM"; then
  echo "Checked locations:"
  echo "  VST3 -> $VST3_DEST"
  echo "  CLAP -> $CLAP_DEST"
  echo "  LV2   -> $LV2_DEST"
  echo "  Standalone -> $BIN_DEST"
fi

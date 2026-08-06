#!/usr/bin/env bash
# Download FreePats and arrange for libmodplug MMPAT_PATH_TO_CFG.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"

DEB_URL="${FREEPATS_DEB_URL:-https://ftp.debian.org/debian/pool/main/f/freepats/freepats_20060219-4_all.deb}"
DEB="$ROOT/freepats.deb"
EXTRACT="$ROOT/deb-extract"

echo "Downloading FreePats..."
curl -L --fail -o "$DEB" "$DEB_URL"

rm -rf "$EXTRACT"
mkdir -p "$EXTRACT"
(
  cd "$EXTRACT"
  ar x "$DEB"
  tar xf data.tar.*
)

SRC="$EXTRACT/usr/share/midi/freepats"
CFGSRC="$EXTRACT/etc/timidity/freepats.cfg"
test -d "$SRC/Tone_000"
test -f "$CFGSRC"

rm -rf "$ROOT/instruments"
mkdir -p "$ROOT/instruments"
cp -R "$SRC/Tone_000" "$SRC/Drum_000" "$ROOT/instruments/"

python3 - "$CFGSRC" "$ROOT/timidity.cfg" <<'PY'
import sys
from pathlib import Path
src, dst = Path(sys.argv[1]), Path(sys.argv[2])
melodic, drums = [], []
mode = "mel"
for line in src.read_text().splitlines():
    s = line.strip()
    if not s or s.startswith("#") or s.startswith("dir "):
        continue
    if s.startswith("drumset"):
        mode = "drum"
        continue
    if s.startswith("bank"):
        mode = "mel"
        continue
    parts = s.split()
    if not parts or not parts[0].isdigit():
        continue
    path = parts[1]
    if path.endswith(".pat"):
        path = path[:-4]
    entry = f"{parts[0]}\t{path}"
    (drums if mode == "drum" else melodic).append(entry)
out = [
    "# ModPlug / libmodplug TiMidity patch map (FreePats 20060219)",
    "# Melodic bank MUST come before drumset — ModPlug never resets isdrumset.",
    "# Paths are relative to $MMPAT_PATH_TO_CFG/instruments and must omit .pat",
    "",
    "bank 0",
    "",
    *melodic,
    "",
    "drumset 0",
    "",
    *drums,
    "",
]
dst.write_text("\n".join(out))
print(f"wrote {dst}: {len(melodic)} melodic, {len(drums)} drums")
PY

# Drop bulky intermediates; keep instruments + cfg.
rm -rf "$EXTRACT" "$DEB"
echo "Ready. export MMPAT_PATH_TO_CFG=\"$ROOT\""

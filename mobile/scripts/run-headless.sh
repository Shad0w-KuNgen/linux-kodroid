#!/usr/bin/env bash
# Başsız (ekransız) istemci testi: N kare çalıştırır, ekran görüntüsünü PNG'ye çevirir.
# Kullanım: scripts/run-headless.sh <veri_dizini> [kare=20] [cikti.png]
set -euo pipefail
DIR="$(cd "$(dirname "$0")/.." && pwd)"
ASSETS="${1:?istemci veri dizini gerekli}"
FRAMES="${2:-20}"
OUT="${3:-$DIR/build/shot.png}"
PPM="${OUT%.png}.ppm"
SDL_VIDEODRIVER=offscreen EGL_PLATFORM=surfaceless LIBGL_ALWAYS_SOFTWARE=1 \
KO_CLIENT_DIR="$ASSETS" KO_MAX_FRAMES="$FRAMES" KO_SCREENSHOT_PHYSICAL="$PPM" \
"$DIR/build/KnightOnLine"
python3 "$DIR/scripts/ppm2png.py" "$PPM" "$OUT"
echo "ekran görüntüsü: $OUT"

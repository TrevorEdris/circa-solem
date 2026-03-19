#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
DATA_DIR="${REPO_ROOT}/data/ephemeris"
DE440_URL="https://ssd.jpl.nasa.gov/ftp/eph/planets/bsp/de440.bsp"
DE440_PATH="${DATA_DIR}/de440.bsp"

mkdir -p "${DATA_DIR}"

if [[ -f "${DE440_PATH}" ]]; then
    echo "de440.bsp already present at ${DE440_PATH}"
    exit 0
fi

echo "Downloading JPL DE440 ephemeris (~83 MB)..."
curl -L --progress-bar -o "${DE440_PATH}" "${DE440_URL}"
echo "Done: ${DE440_PATH}"

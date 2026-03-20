#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
TEXTURE_DIR="${REPO_ROOT}/assets/textures"

mkdir -p "${TEXTURE_DIR}"

# Helper: download a file if not already present.
download() {
    local url="$1"
    local filename="$2"
    local desc="$3"
    local path="${TEXTURE_DIR}/${filename}"

    if [[ -f "${path}" ]]; then
        echo "${filename} already present"
        return
    fi

    echo "Downloading ${desc} ..."
    curl -L --progress-bar -o "${path}" "${url}"

    # Verify we got a real image, not an HTML error page
    if head -1 "${path}" | grep -q "<!DOCTYPE\|<html"; then
        echo "WARNING: ${filename} is an HTML error page, not an image. Removing."
        rm -f "${path}"
        echo "  You may need to manually download from: ${url}"
        return 1
    fi

    # Verify file is non-trivial (> 1KB)
    local size
    size=$(wc -c < "${path}" | tr -d ' ')
    if [[ "${size}" -lt 1024 ]]; then
        echo "WARNING: ${filename} is suspiciously small (${size} bytes). May not be valid."
    fi

    echo "Done: ${path}"
}

# Saturn ring texture from Solar System Scope (CC BY 4.0)
# If this URL doesn't work, visit https://www.solarsystemscope.com/textures/
# and download the Saturn Ring texture manually to assets/textures/saturn_ring.png
download \
    "https://www.solarsystemscope.com/textures/download/2k_saturn_ring_alpha.png" \
    "saturn_ring.png" \
    "Saturn ring texture (Solar System Scope)" \
    || echo "  Fallback: program will generate a procedural ring texture"

# Uranus and Neptune rings are very faint — procedural textures are sufficient.
# If you want real textures, download them manually from:
#   https://planetpixelemporium.com/uranus.html
#   https://www.solarsystemscope.com/textures/

echo ""
echo "Ring textures downloaded to ${TEXTURE_DIR}"
echo "Note: Uranus and Neptune use procedural ring textures (real rings are too faint for photographic detail)"

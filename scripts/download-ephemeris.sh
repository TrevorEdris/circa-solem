#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
DATA_DIR="${REPO_ROOT}/data/ephemeris"

mkdir -p "${DATA_DIR}"

# Helper: download a file if not already present.
download() {
    local url="$1"
    local filename="$2"
    local desc="$3"
    local path="${DATA_DIR}/${filename}"

    if [[ -f "${path}" ]]; then
        echo "${filename} already present"
        return
    fi

    echo "Downloading ${desc} ..."
    curl -L --progress-bar -o "${path}" "${url}"

    # Verify we didn't get an HTML error page
    if head -1 "${path}" | grep -q "<!DOCTYPE\|<html"; then
        echo "ERROR: ${filename} is an HTML error page, not a binary kernel. Removing."
        rm -f "${path}"
        return 1
    fi

    echo "Done: ${path}"
}

# Planetary ephemeris (planets + Moon + Pluto barycenter)
download \
    "https://ssd.jpl.nasa.gov/ftp/eph/planets/bsp/de440.bsp" \
    "de440.bsp" \
    "JPL DE440 planetary ephemeris (~114 MB)"

# Satellite ephemerides (moons relative to planet barycenters)
NAIF_SAT="https://naif.jpl.nasa.gov/pub/naif/generic_kernels/spk/satellites"

download "${NAIF_SAT}/jup365.bsp" "jup365.bsp" \
    "Jupiter satellite ephemeris (Io, Europa, Ganymede, Callisto) (~1.1 GB)"

download "${NAIF_SAT}/sat441.bsp" "sat441.bsp" \
    "Saturn satellite ephemeris (Titan, Rhea) (~631 MB)"

download "${NAIF_SAT}/ura111xl-703.bsp" "ura111xl-703.bsp" \
    "Uranus satellite ephemeris (Titania)"

download "${NAIF_SAT}/ura111xl-704.bsp" "ura111xl-704.bsp" \
    "Uranus satellite ephemeris (Oberon)"

download "${NAIF_SAT}/nep097.bsp" "nep097.bsp" \
    "Neptune satellite ephemeris (Triton)"

download "${NAIF_SAT}/plu060.bsp" "plu060.bsp" \
    "Pluto satellite ephemeris (Charon)"

# Small body ephemerides (Ceres and numbered asteroids)
download \
    "https://ssd.jpl.nasa.gov/ftp/eph/small_bodies/asteroids_de441/sb441-n16.bsp" \
    "sb441-n16.bsp" \
    "Small body ephemeris — 16 largest asteroids including Ceres (~616 MB)"

echo ""
echo "All ephemeris files present in ${DATA_DIR}"

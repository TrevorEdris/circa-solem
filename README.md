# circa-solem

> *"Around the Sun"* — A real-time solar system simulation. Art piece and interactive sandbox.

Built with C++20, OpenGL 4.1, GLFW, GLM, and CMake.

---

## Prerequisites

### macOS

```bash
brew install cmake glfw glm
```

GLAD (OpenGL 4.1 Core loader) and stb_image are pre-generated and committed to `extern/` — no additional setup needed.

### Windows

CMake fetches GLFW and GLM automatically on first configure. No manual installation required.

Requires Visual Studio 2022 (or Build Tools) with the C++ workload.

---

## Build

```bash
cmake -B build
cmake --build build
./build/bin/circa-solem
```

---

## Ephemeris Data (required for Phase 2+)

Body positions from Phase 2 onward use the JPL DE440 ephemeris (~83 MB). Download it once before running:

```bash
# macOS / Linux
bash scripts/download-ephemeris.sh

# Windows (PowerShell)
.\scripts\download-ephemeris.ps1
```

The file is saved to `data/ephemeris/de440.bsp` and excluded from version control.

---

## Project Structure

```
src/                   Application source
include/circa-solem/   Public headers
shaders/               GLSL shader files
extern/glad/           OpenGL 4.1 Core loader (pre-generated, committed)
extern/stb/            stb_image single-header library
assets/                Texture maps and models (large files gitignored)
data/                  Ephemeris and runtime data (gitignored)
scripts/               Setup and utility scripts
tests/                 Test suite (CTest)
docs/planning/         Design and planning documents
```

---

## Controls

| Key | Action |
|-----|--------|
| ESC | Quit |

*(More controls added in Phase 1+)*

# Circa Solem — Feature Roadmap

High-level plan for all systems and features. Each entry describes the feature, its dependencies, and which development phase it belongs to. Implementation specs will live in `specs/<category>/` when created.

Source material: `docs/planning/claude-chat-raw/`

---

## Phase 0 — Scaffolding

### P0-A: CMake Project Setup
- **What:** Complete CMakeLists.txt, directory layout, and cross-platform build configuration.
- **Layout (Pitchfork-inspired):**
  - `src/` — C++ source files, organized by subsystem
  - `include/circa-solem/` — public headers
  - `shaders/` — GLSL vertex/fragment shader files
  - `assets/` — textures, models (large files git-ignored or LFS)
  - `data/` — ephemeris data (git-ignored; populated by setup script)
  - `extern/` — vendored third-party code (GLAD generated files)
  - `scripts/` — helper scripts (ephemeris download, asset conversion)
  - `tests/` — numerical validation tests
  - `docs/` — planning and design documentation
- **Toolchain:** CMake 3.25+. Targets `CXX_STANDARD 20`.
- **Phase:** 0
- **Depends on:** Nothing

### P0-B: Dependency Integration (GLFW, GLAD, GLM)
- **What:** Integrate the three core dependencies in a way that works on macOS (brew) and Windows 11 (no brew).
- **Strategy:** `find_package` first (satisfies brew installs on Mac), with `FetchContent` fallback (satisfies Windows where no system install exists). GLAD is generated for OpenGL 4.1 Core Profile and committed to `extern/glad/` — it's a small pair of C files, not a library.
  - GLFW: `find_package(glfw3) → FetchContent 3.4`
  - GLM: `find_package(glm) → FetchContent 1.0.1`
  - GLAD: `extern/glad/` (committed, generated once at project init for GL 4.1 Core, no-extensions)
- **Phase:** 0
- **Depends on:** P0-A

### P0-C: First Window
- **What:** A GLFW window opens with an OpenGL 4.1 Core context. Clears to a dark color. Exits cleanly on ESC.
- **Verification:** Window opens on macOS and Windows without errors. OpenGL error callback registered. GLFW error callback registered.
- **Phase:** 0
- **Depends on:** P0-B

### P0-D: Ephemeris Data Setup
- **What:** `scripts/download-ephemeris.sh` (and `.ps1` for Windows) downloads DE440 (and optionally DE441) from JPL's FTP servers into `data/ephemeris/`. `data/` is in `.gitignore`. README documents the setup step.
- **JPL source:** `https://ssd.jpl.nasa.gov/ftp/eph/planets/`
- **Phase:** 0
- **Depends on:** P0-A

### P0-E: Shader Loading Infrastructure
- **What:** Runtime GLSL shader compiler. Reads `.vert`/`.frag` files from `shaders/`, compiles and links into a `ShaderProgram` object. Reports compile/link errors with file name and line number.
- **Phase:** 0
- **Depends on:** P0-C

---

## Phase 1 — Engine Foundation

### P1-A: Sphere Renderer
- **What:** Renders a single lit sphere in 3D space using Phong shading (ambient, diffuse, specular). Geometry is a UV sphere generated procedurally with configurable resolution. Rendered with VAO/VBO.
- **Phase:** 1
- **Depends on:** P0-E
- **Risk:** UV sphere subdivision resolution needs tuning — too low looks chunky close-up, too high wastes geometry on distant bodies.

### P1-B: Orbit Camera
- **What:** 3D perspective camera with orbit (mouse drag), zoom (scroll wheel), and pan (middle-mouse drag or Shift+drag). Implemented as arcball-style rotation around a focus point.
- **Phase:** 1
- **Depends on:** P0-C

### P1-C: Procedural Starfield
- **What:** Background of randomly distributed point stars rendered on a large sphere or as a skybox. Point size and brightness vary slightly. Depth writes disabled so it renders behind everything.
- **Note:** Replaced with real Hipparcos catalog data in Phase 4.
- **Phase:** 1
- **Depends on:** P0-E

### P1-D: Velocity Verlet Integrator
- **What:** Symplectic numerical integrator computing gravitational acceleration for N bodies and integrating positions and velocities. Chosen over Euler (unstable for orbits) and RK4 (non-symplectic, energy drift over long runs).
- **Formula:** `F = Gm₁m₂/r²` applied to all body pairs.
- **Phase:** 1
- **Depends on:** Nothing
- **Risk:** Numerical precision at solar system mass/distance scales requires careful unit normalization (AU, solar masses, years rather than meters/kg/seconds).

### P1-E: Body Entity System
- **What:** Core data model separating `SimulatedBody` (gravitationally integrated) from `DecorativeBody` (prescribed path, no physics). Both render as spheres. Both have: name, mass, radius, position, velocity, color. `DecorativeBody` additionally has an orbital path function.
- **Phase:** 1
- **Depends on:** P1-D

### P1-F: Configurable Time-Step Loop
- **What:** Main simulation loop with a configurable `delta_t`. Not a fixed-rate game loop — `delta_t` can be scaled freely. Designed from day one for time warp (Phase 2) and arbitrary date (Phase 2).
- **Phase:** 1
- **Depends on:** P1-D

### P1-G: Simulation Clock
- **What:** Tracks the simulation's current date/time as a Julian Date. Supports arbitrary start date (not a forward-only wall clock). The integration loop advances this clock by `delta_t` per step.
- **Phase:** 1
- **Depends on:** P1-F

### P1-H: Two-Body Stable Orbit Demo
- **What:** Sun + one planet (or planet + moon). Planet orbits stably using simplified Keplerian initial conditions. No spiral-in or escape. Demonstrates that the integrator, entity system, and clock all work together.
- **Note:** Real ephemeris data does not arrive until Phase 2. Initial conditions are hand-tuned.
- **Phase:** 1
- **Depends on:** P1-A, P1-B, P1-C, P1-D, P1-E, P1-F, P1-G
- **Milestone:** *A moon orbits a planet stably. Camera can orbit and zoom. Nothing else.*

---

## Phase 2 — Inner Solar System

### P2-A: JPL Ephemeris Integration
- **What:** Integrate Bill Gray's `jpl_eph` C library to read DE440 binary files and evaluate body state vectors (position + velocity) at any Julian Date. Wrapped in a C++ `EphemerisProvider` class.
- **Library:** `https://github.com/Bill-Gray/jpl_eph`
- **Integration:** CMake `FetchContent` or git submodule in `extern/jpl_eph/`.
- **Phase:** 2
- **Depends on:** P0-D (data present), P1-G (Julian Date clock)
- **Risk:** Binary file format is endian-sensitive and version-sensitive. Test on macOS and Windows early.

### P2-B: Inner Planet Bodies
- **What:** Data for Sun, Mercury, Venus, Earth, Mars. Each has mass, physical radius, and initial state vector from DE440.
- **Phase:** 2
- **Depends on:** P2-A, P1-E

### P2-C: Luna (First Tier 1 Moon)
- **What:** Earth's Moon as the first gravitationally simulated moon. State vector from DE440. Visually distinct from Earth.
- **Phase:** 2
- **Depends on:** P2-B

### P2-D: Scale System
- **What:** Two independent scale factors: `sizeScale` (planet radius multiplier) and `distanceScale` (orbital distance multiplier). UI toggle for "True Scale" preset (both = 1.0) and "Display Scale" preset (sizeScale exaggerated, distances compressed proportionally). Real-time adjustable.
- **Phase:** 2
- **Depends on:** P1-A, P1-B

### P2-E: Time Warp
- **What:** Controls to multiply the simulation's `delta_t`: 1×, 10×, 100×, 1,000×, 10,000×, 100,000×, forward and reverse. Pause. Default rate tuned so Earth orbits in a few minutes at display speed.
- **Phase:** 2
- **Depends on:** P1-F, P1-G

### P2-F: Orbit Trails
- **What:** Fading line traces of each body's recent path. Implemented as a ring buffer of recent positions per body, drawn as a GL_LINE_STRIP with alpha falloff.
- **Phase:** 2
- **Depends on:** P1-A

### P2-G: Basic Sun Glow
- **What:** Additive billboard sprite or simple bloom pass on the Sun giving it a visible glow. Not the full HDR bloom from Phase 4 — just enough to make the Sun feel luminous.
- **Phase:** 2
- **Depends on:** P1-A, P0-E

### P2-H: Numerical Validation — Inner Planets
- **What:** Compare computed Earth (and optionally Mars, Venus) positions against JPL Horizons output for 3+ known dates. Error threshold: < 1% of orbital radius.
- **Tool:** JPL Horizons API (`https://ssd.jpl.nasa.gov/horizons/`) generates reference position tables.
- **Phase:** 2
- **Depends on:** P2-B, P2-C
- **Milestone:** *Inner solar system with stable, visually distinct orbits. Time warp works. Scale feels right.*

---

## Phase 3 — Full Solar System

### P3-A: Outer Planet Bodies
- **What:** Jupiter, Saturn, Uranus, Neptune — mass, radius, state vectors from DE440. All gravitationally simulated.
- **Phase:** 3
- **Depends on:** P2-A, P1-E

### P3-B: Dwarf Planet Bodies
- **What:** Pluto, Ceres, Eris, Haumea, Makemake — gravitationally simulated. State vectors from DE440 where available; Keplerian elements as fallback for dwarf planets outside DE440's primary target set.
- **Phase:** 3
- **Depends on:** P2-A, P1-E

### P3-C: Tier 1 Moons (Gravitationally Simulated)
- **What:** ~10–12 moons with full N-body integration: Io, Europa, Ganymede, Callisto (Jupiter); Titan, Rhea (Saturn); Triton (Neptune); Charon (Pluto); Titania, Oberon (Uranus). State vectors from DE440.
- **Phase:** 3
- **Depends on:** P3-A, P3-B

### P3-D: Tier 2 Moons (Named Decorative)
- **What:** ~50–70 named moons on prescribed orbital paths (Keplerian elements, no gravitational integration). Phobos, Deimos, Enceladus, Miranda, Hyperion, Nereid, and others. Clickable in Phase 5 for info panels.
- **Phase:** 3
- **Depends on:** P1-E

### P3-E: Tier 3 Moon Swarms
- **What:** Hundreds of anonymous moon particles procedurally scattered in correct orbital zones for the gas giants. Rendered with instanced sphere drawing. No individual identity — swarm behavior only.
- **Technical:** `glDrawArraysInstanced` with per-instance position/size VBO. CPU-side position update each frame (25 Tier 1 + a few hundred swarm particles is trivial at CPU).
- **Phase:** 3
- **Depends on:** P1-A

### P3-F: Saturn's Rings
- **What:** Textured flat disc geometry (hollow torus or two triangles) around Saturn. Not physically simulated. Texture: grayscale opacity map for ring density variation.
- **Phase:** 3
- **Depends on:** P3-A

### P3-G: Uranus & Neptune Ring Systems
- **What:** Subtle ring geometry, thinner and less opaque than Saturn's. Same approach as P3-F.
- **Phase:** 3
- **Depends on:** P3-A

### P3-H: Asteroid Belt
- **What:** Decorative particle band between Mars and Jupiter. Instanced rendering, elliptical distribution approximating the real belt shape. ~2,000–5,000 particles.
- **Phase:** 3
- **Depends on:** P1-A

### P3-I: Kuiper Belt
- **What:** Decorative particle band beyond Neptune in the 30–55 AU zone. Same approach as asteroid belt, lower density.
- **Phase:** 3
- **Depends on:** P1-A

### P3-J: Basic Cinematic Autopilot
- **What:** Simple autonomous camera that drifts between planets on a timer — not authored, just random/sequential body visits. Enough to leave the simulation running on a screen without manual camera input. Replaced by curated autopilot in Phase 5.
- **Phase:** 3
- **Depends on:** P1-B
- **Milestone:** *The complete solar system is present. Gas giant systems feel alive with moon swarms. Belts and rings provide visual context. You can leave it running on a screen and it looks beautiful.*

---

## Phase 4 — Visual Polish

### P4-A: Planet Texture Mapping
- **What:** Equirectangular texture maps applied to all planets and major moons. Source: Solar System Scope free texture pack as primary; NASA JPL maps as supplementary.
- **Technical:** Sphere UV coordinates generated at build time. Texture loaded via `stb_image`. `GL_TEXTURE_2D` with mipmaps.
- **Phase:** 4
- **Depends on:** P1-A

### P4-B: Texture LOD System
- **What:** Two texture sizes per body: low-res (distant) and high-res (close zoom). System selects based on camera distance to the body. Texture-only (no geometry LOD).
- **Phase:** 4
- **Depends on:** P4-A

### P4-C: Earth Special Layers
- **What:** Three additional Earth texture layers composited in the fragment shader: cloud layer (semi-transparent), city lights (visible only on the dark side), ocean specular reflection (specular map masking land).
- **Phase:** 4
- **Depends on:** P4-A

### P4-D: Gas Giant Cloud Animation
- **What:** Animated banded cloud texture for Jupiter and Saturn (and subtly Uranus/Neptune). UV offset or noise-driven distortion on the cloud texture, very slow movement.
- **Phase:** 4
- **Depends on:** P4-A

### P4-E: Real Star Catalog (Hipparcos)
- **What:** Replace procedural starfield with the Hipparcos catalog (~118,000 stars). Stars positioned correctly on the celestial sphere. Point size and brightness derived from catalog magnitude values.
- **Source:** Pre-formatted JSON by magnitude from `https://www.celestialprogramming.com/starcatalog/starcatalogs.html`
- **Phase:** 4
- **Depends on:** P1-C (replaces it)

### P4-F: HDR Rendering Pipeline
- **What:** Framebuffer with floating-point color attachment. Tone mapping (Reinhard or ACES) applied in a post-process pass. Required foundation for correct bloom and atmospheric glow.
- **Phase:** 4
- **Depends on:** P0-E

### P4-G: Sun Bloom (Physically-Based)
- **What:** Extract bright regions from the HDR framebuffer, apply Gaussian blur with progressive downsampling/upsampling (LearnOpenGL physically-based bloom approach), composite back. Replaces the simple glow from P2-G.
- **Phase:** 4
- **Depends on:** P4-F

### P4-H: Atmospheric Scattering
- **What:** Rayleigh + Mie scattering on Earth, Venus, Titan. Fragment shader computes light wrapping around the planet limb. Starting point: `wwwtyro/glsl-atmosphere`, adapted to OpenGL 4.1 Core.
- **Phase:** 4
- **Depends on:** P4-F
- **Risk:** Per-planet atmosphere parameters (radius, density, scattering coefficients) need tuning per body. Venus is very thick; Titan is hazy orange.

### P4-I: Terminator Line
- **What:** Soft day/night boundary on all bodies. Fragment shader computes the dot product of the surface normal with the Sun direction and applies a smooth gradient in the penumbra zone.
- **Phase:** 4
- **Depends on:** P4-A

### P4-J: God Rays
- **What:** Radial blur / volumetric scattering effect when the Sun is occluded by a planet. Screen-space radial blur originating from the projected Sun position.
- **Phase:** 4
- **Depends on:** P4-F

### P4-K: Shadow Mapping (Point Shadows)
- **What:** Cubemap-based shadow mapping with the Sun as the point light source. Required foundation for eclipses (P4-L through P4-N).
- **Technical:** OpenGL 4.1 compatible. Render scene depth into a cubemap from the Sun's position; use during main pass to determine shadow.
- **Phase:** 4
- **Depends on:** P4-F

### P4-L: Solar Eclipses
- **What:** The Moon's shadow cone visible on Earth's surface during a solar eclipse. Uses shadow mapping (P4-K).
- **Phase:** 4
- **Depends on:** P4-K, P2-C

### P4-M: Lunar Eclipses
- **What:** Earth's shadow crossing the Moon. The Moon dims and takes on a reddish tone (Rayleigh-scattered light refracted through Earth's atmosphere).
- **Phase:** 4
- **Depends on:** P4-K, P4-H, P2-C

### P4-N: Jupiter Moon Shadow Dots
- **What:** Galilean moon shadows as small dark dots on Jupiter's cloud bands.
- **Phase:** 4
- **Depends on:** P4-K, P3-C

### P4-O: Lens Flare (Optional Toggle)
- **What:** Screen-space lens flare streaks and halos when the Sun is in frame. Off by default. Toggle in settings.
- **Phase:** 4
- **Depends on:** P4-F
- **Milestone:** *Someone walks past the screen and stops to stare. The lighting is beautiful. An eclipse happens and it's breathtaking.*

---

## Phase 5 — Interaction & Time Travel

### P5-A: Curated Cinematic Autopilot
- **What:** Upgrade Phase 3's basic autopilot with authored vantage points: a sunrise over Saturn's rings, cruising alongside Europa with Jupiter looming, a slow pull-back from Earth to reveal the inner system. Smooth eased camera transitions between points. Loops indefinitely.
- **Phase:** 5
- **Depends on:** P3-J (replaces it)

### P5-B: Click-to-Follow Camera
- **What:** Click any rendered body to have the camera smoothly transition to tracking it. Camera stays focused on the selected body as it moves.
- **Phase:** 5
- **Depends on:** P1-B, body selection picking

### P5-C: Lock-To Orbit Mode
- **What:** Camera orbits a selected body at a fixed distance. Controls become orbit/zoom relative to the locked body rather than the origin.
- **Phase:** 5
- **Depends on:** P5-B

### P5-D: Free-Fly Camera
- **What:** WASD + mouse look flight through the solar system. No focus point.
- **Phase:** 5
- **Depends on:** P1-B

### P5-E: Smooth Camera Mode Transitions
- **What:** Eased interpolation when switching between orbit, lock-to, free-fly, and autopilot modes. No jarring jump cuts.
- **Phase:** 5
- **Depends on:** P5-A, P5-B, P5-C, P5-D

### P5-F: Body Selection & Info Panel
- **What:** Click a body to select it; a minimal info panel fades in showing name, mass, radius, current velocity, distance from Sun, orbital period. Panel hidden in art piece mode (no selection active).
- **Phase:** 5
- **Depends on:** P1-E

### P5-G: Minimal UI Framework
- **What:** All UI elements are hidden by default. Fade in on mouse move/interaction; fade out after inactivity. Preserves the art piece aesthetic during passive display.
- **Phase:** 5
- **Depends on:** P0-E

### P5-H: Date Picker
- **What:** Jump to any arbitrary date within the DE440/DE441 range. Input as calendar date; converted to Julian Date internally. Body positions re-evaluated from ephemeris at the target date.
- **Phase:** 5
- **Depends on:** P2-A, P1-G

### P5-I: Playback Speed Dial
- **What:** Continuous-range speed control from reversed centuries-per-second to forward centuries-per-second. Different from the discrete multipliers in P2-E — this is a draggable dial for the time-travel UX.
- **Phase:** 5
- **Depends on:** P2-E

### P5-J: Ephemeris Confidence Indicator
- **What:** Subtle visual cue (color tint on the date display) as the simulation date moves outside the high-accuracy range of DE440 (1550–2650). Transitions: accurate → approximate → projected.
- **Phase:** 5
- **Depends on:** P5-H

### P5-K: Date/Time Overlay
- **What:** Always-visible minimal overlay showing the current simulation date (and real-world elapsed time if paused). Small, positioned in a corner, styled to not distract from the art piece.
- **Phase:** 5
- **Depends on:** P1-G, P5-G

### P5-L: Curated Event Library
- **What:** Predefined events that jump to a specific date and position the camera cinematically on the relevant action.
  - **Historic:** Apollo 11 trajectory, Voyager 1 & 2 flybys, Shoemaker-Levy 9 (1994), New Horizons Pluto flyby (2015), Cassini Saturn arrival
  - **Eclipses:** Total solar eclipses (including 2024 North American), lunar eclipses, Transit of Venus (2117), Transit of Mercury
  - **Milestones:** Grand Tour alignment (Voyager era), planetary conjunctions, perihelion/aphelion dates
- **Phase:** 5
- **Depends on:** P5-H, P5-A

### P5-M: Eclipse Frequency Tuning
- **What:** Orbital inclinations may be subtly adjusted in "display mode" so eclipses occur more frequently than in reality. "True accuracy" toggle restores real inclinations. Deferred from Phase 4 to avoid scope during the shader-heavy polish phase.
- **Phase:** 5
- **Depends on:** P4-L, P4-M

### P5-N: Numerical Validation — Event Library Dates
- **What:** Verify computed body positions at 3+ curated event dates against JPL Horizons. Error threshold: < 1% of orbital radius. Confirms the time-travel system is historically accurate.
- **Phase:** 5
- **Depends on:** P5-H, P5-L
- **Milestone:** *You can leave it running in autopilot mode all day, or sit down and explore the solar system on the date of the Moon landing with the camera framing the Earth-Moon system.*

---

## Phase 6 — Sandbox & Extras

### P6-A: Custom Body Spawning
- **What:** UI to spawn a new body with user-defined mass, radius, initial position (click to place), and initial velocity (drag to set direction/magnitude). Body enters the N-body simulation immediately.
- **Phase:** 6
- **Depends on:** P1-E, P5-F

### P6-B: Collision Detection & Response
- **What:** Sphere-sphere collision detection between all simulated bodies. Response: bodies merge (sum of masses, momentum-conserving velocity) or shatter (multiple smaller fragments) based on relative velocity threshold.
- **Phase:** 6
- **Depends on:** P1-D, P6-A

### P6-C: Chaos Mode
- **What:** Spawn N equal-mass stars distributed throughout the scene and watch N-body gravitational chaos unfold. Preset button. Bodies interact gravitationally; collisions enabled.
- **Phase:** 6
- **Depends on:** P6-A, P6-B

### P6-D: Comet Bodies
- **What:** Custom comet-like bodies with highly elongated (high-eccentricity) orbits. Visual tail (particle sprite trail) oriented away from the Sun. Can be spawned in the sandbox or added as named pre-configured comets.
- **Phase:** 6
- **Depends on:** P6-A

### P6-E: Gravitational Field Visualization
- **What:** Toggleable overlay showing the gravitational influence field as a warped grid (space-time curvature metaphor) or a heat-map color field. Computed on CPU for simplicity.
- **Phase:** 6
- **Depends on:** P1-D

### P6-F: Gravitational Lensing Effect
- **What:** Screen-space shader distortion around massive bodies simulating gravitational lensing. Subtle on planets, more pronounced in chaos mode with massive bodies.
- **Phase:** 6
- **Depends on:** P4-F

### P6-G: Mini-Map Overlay
- **What:** Top-down 2D orbital diagram as a small corner overlay. Shows all body positions as dots with the Sun at center. Click a dot to select the body. Toggled with a key.
- **Phase:** 6
- **Depends on:** P5-F

### P6-H: Spacecraft Models (Decorative)
- **What:** Small 3D models or sprites on known trajectories: Hubble (LEO), JWST (Sun-Earth L2), Voyager 1 & 2 (heliocentric escape), New Horizons, Kepler, Chandra. Models from NASA 3D Resources (public domain).
- **Technical:** Decorative bodies using `DecorativeBody` path functions. Trajectory data from JPL Horizons or SPICE kernels.
- **Phase:** 6
- **Depends on:** P1-E (decorative pipeline), P5-L (some spacecraft at curated event positions)

### P6-I: SPICE Trajectory Data (Spacecraft)
- **What:** Integrate CSPICE (NASA JPL C toolkit) to provide precise spacecraft trajectory data for Voyager 1 & 2 (currently ~24 billion km from Earth) and other missions with available SPK kernels.
- **Library:** `https://naif.jpl.nasa.gov/naif/toolkit.html`
- **Phase:** 6
- **Depends on:** P6-H

### P6-J: Personal Date Bookmarks *(Stretch Goal)*
- **What:** Save any current simulation date with a custom label (e.g., "The night my kid was born"). Persisted to a local config file. Accessible from the event library alongside curated events.
- **Phase:** 6 (stretch)
- **Depends on:** P5-L

---

## Explicit Non-Goals

- **Sound/audio** — space is silent.
- **Multiplayer or networking**
- **VR support**
- **Game mechanics** — no scoring, progression, or objectives.
- **Procedural fictional solar systems** — this is *our* solar system.
- **Heavy default UI** — HUD is hidden in art piece mode.

---

## Cross-Cutting Concerns

### OpenGL Version
- Target: OpenGL 4.1 Core Profile everywhere (macOS cap).
- Windows dev/testing uses 4.1 for consistency, even though 4.6 is available.
- No compute shaders, no SPIR-V, no `imageLoad`/`imageStore`. All particle systems handled via CPU-updated instanced rendering.

### Performance Budget
- Simulated bodies: ~25 (Sun + 8 planets + 12 Tier 1 moons + 5 dwarf planets). N-body O(n²) at this scale is trivial on any modern CPU.
- Decorative instanced bodies: ~2,000–5,000 (belts + Tier 3 swarms). All instanced rendering.
- Target: stable 60 fps on M-series Mac at 2560×1600. Shader-heavy Phase 4 effects may require tuning on Mac; dedicated GPU (RTX 5080) on Windows provides headroom.

### Platform Matrix
| Phase | macOS (M-series) | Windows 11 (RTX 5080) |
|-------|------------------|------------------------|
| 0–3   | Primary dev target | Implicit (FetchContent handles deps) |
| 4     | Development | Windows build verification milestone (shader features) |
| 5+    | Primary dev target | Tested when GPU power needed |

### Dependency Summary
| Library | Source (Mac) | Source (Windows) |
|---------|-------------|-----------------|
| GLFW 3.4 | `brew install glfw` or FetchContent | FetchContent |
| GLM 1.0.1 | `brew install glm` or FetchContent | FetchContent |
| GLAD (GL 4.1 Core) | `extern/glad/` (committed) | `extern/glad/` (committed) |
| jpl_eph | FetchContent / submodule | FetchContent / submodule |
| stb_image | `extern/stb/` (single-header, committed) | same |
| CSPICE (Phase 6) | Download from NAIF | Download from NAIF |

### Numerical Validation Strategy
- **Tool:** JPL Horizons API for reference positions at known dates.
- **Target bodies:** Earth, Mars (Phase 2); Jupiter, Saturn, Triton (Phase 3); all curated event positions (Phase 5).
- **Threshold:** < 1% of body's orbital semi-major axis at each test date.
- **Execution:** Manual during development. Stretch goal: automated regression test in `tests/`.

# Circa Solem — Development Plan

## Current Status

**Phase:** 0 — Scaffolding
**Last Updated:** 2026-03-19
**Playable Milestone:** Nothing yet — project is pre-code.

### Completed
*(none)*

### In Progress
- [ ] Phase 0 scaffolding

---

## Philosophy

Build in vertical slices. Each phase produces something visible and runnable. Physics correctness is validated before visual layers depend on it. Every session should end with something you can launch and see. Visual polish comes after functional correctness — a blinking sphere with correct orbits is worth more than a beautiful screenshot with wrong physics.

Source: `docs/planning/claude-chat-raw/Circa_Solem_Feature_Specification.md`

---

## Phase 0: Scaffolding

**Entry Criteria:** Empty repository.
**Exit Criteria:** Project builds and runs on macOS (brew). A blank GLFW window opens with an OpenGL 4.1 context. Dependencies resolve. Ephemeris data download is documented.

### Checklist

- [ ] **P0-A:** CMake project scaffolding — `feature_roadmap.md §P0-A`
  - [ ] `CMakeLists.txt` at root targeting C++20, OpenGL 4.1 Core
  - [ ] Directory layout: `src/`, `include/circa-solem/`, `shaders/`, `extern/`, `assets/`, `data/`, `scripts/`, `tests/`, `docs/`
  - [ ] `.gitignore` covering `build/`, `data/`, large asset files
  - [ ] `cmake/` directory for helper modules (FindPackage overrides, utility functions)
- [ ] **P0-B:** Dependency integration — `feature_roadmap.md §P0-B`
  - [ ] GLAD generated for OpenGL 4.1 Core Profile (no extensions), committed to `extern/glad/`
  - [ ] GLFW: `find_package(glfw3 3.3 QUIET)` → `FetchContent` fallback, tag 3.4
  - [ ] GLM: `find_package(glm QUIET)` → `FetchContent` fallback, tag 1.0.1
  - [ ] `stb_image.h` committed to `extern/stb/` (single-header, no build step)
  - [ ] README documents: `brew install glfw glm cmake` for Mac; CMake auto-fetches on Windows
- [ ] **P0-C:** First window — `feature_roadmap.md §P0-C`
  - [ ] `src/main.cpp` opens a GLFW window with OpenGL 4.1 Core context
  - [ ] Clears to `vec3(0.05, 0.05, 0.1)` (dark space blue) each frame
  - [ ] Exits cleanly on ESC or window close
  - [ ] GLFW error callback registered and printing to stderr
  - [ ] OpenGL debug message callback registered (if `GL_KHR_debug` available, else no-op)
  - [ ] Verify: builds and runs on macOS (M-series) without warnings
- [ ] **P0-D:** Ephemeris data setup — `feature_roadmap.md §P0-D`
  - [ ] `scripts/download-ephemeris.sh` downloads DE440 from JPL FTP to `data/ephemeris/`
  - [ ] `scripts/download-ephemeris.ps1` Windows equivalent
  - [ ] README setup section documents the download step as a prerequisite
  - [ ] `data/` confirmed in `.gitignore`
- [ ] **P0-E:** Shader loading infrastructure — `feature_roadmap.md §P0-E`
  - [ ] `ShaderProgram` class: loads `.vert`/`.frag` from `shaders/`, compiles, links
  - [ ] Error output includes filename and line number from the GLSL compiler log
  - [ ] Hot-reload-friendly: `ShaderProgram::reload()` method (useful during development)
  - [ ] First shader: trivial passthrough (renders nothing useful, just proves the pipeline)

### Deliverable
Clone the repo → `brew install glfw glm cmake` → `cmake -B build && cmake --build build` → window opens with a dark background. On Windows: `cmake -B build` fetches GLFW/GLM automatically.

---

## Phase 1: Engine Foundation

**Entry Criteria:** Phase 0 complete. Window opens, shader pipeline works.
**Exit Criteria:** A moon orbits a planet stably using Velocity Verlet integration. Camera orbits the scene. Procedural starfield background visible.

### Checklist

- [ ] **P1-A:** Sphere renderer — `feature_roadmap.md §P1-A`
  - [ ] `Sphere` geometry class: generates UV sphere with configurable lat/lon subdivisions
  - [ ] VAO/VBO/EBO uploaded to GPU. Vertex attributes: position, normal, UV.
  - [ ] `PhongShader`: ambient + diffuse + specular, single directional light
  - [ ] Renders one sphere at a given position/radius/color
  - [ ] Verify: sphere visible, shading changes as camera moves
- [ ] **P1-B:** Orbit camera — `feature_roadmap.md §P1-B`
  - [ ] Arcball-style orbit: left-click drag rotates around focus point
  - [ ] Scroll wheel zooms (clamp min/max distance)
  - [ ] Middle-click or Shift+drag pans focus point
  - [ ] Perspective projection, FOV 45°, near/far planes appropriate for solar system scale
- [ ] **P1-C:** Procedural starfield — `feature_roadmap.md §P1-C`
  - [ ] ~5,000 randomly distributed point stars on a large sphere
  - [ ] Slight brightness and size variation per star
  - [ ] Depth writes disabled; renders behind all other geometry
  - [ ] Replaced in Phase 4 (P4-E) — this is a placeholder
- [ ] **P1-D:** Velocity Verlet integrator — `feature_roadmap.md §P1-D`
  - [ ] `Integrator` class with `step(bodies, dt)` method
  - [ ] Computes gravitational acceleration for all simulated body pairs: `F = G*m1*m2 / r²`
  - [ ] Velocity Verlet update: `x(t+dt) = x(t) + v(t)*dt + 0.5*a(t)*dt²`; `v(t+dt) = v(t) + 0.5*(a(t)+a(t+dt))*dt`
  - [ ] Units: AU for distance, solar masses for mass, years for time (G = 4π² in these units)
  - [ ] Softening parameter `ε` to prevent singularity at near-zero separation
  - [ ] Verify: single planet in circular orbit, energy conserved over 1000+ steps
- [ ] **P1-E:** Body entity system — `feature_roadmap.md §P1-E`
  - [ ] `BodyType` enum: `SIMULATED`, `DECORATIVE`
  - [ ] `Body` struct: name, mass, physical radius (km), position (AU), velocity (AU/yr), color, body type
  - [ ] `DecorativeBody` extension: orbital path function `position(julianDate) → vec3`
  - [ ] `BodyRegistry` holds all bodies; integrator operates only on `SIMULATED` bodies
- [ ] **P1-F:** Configurable time-step loop — `feature_roadmap.md §P1-F`
  - [ ] Simulation loop separate from render loop
  - [ ] `SimClock` tracks elapsed simulation time (Julian Date)
  - [ ] `delta_t` exposed as a settable multiplier (time warp built on this in P2-E)
  - [ ] Fixed physics sub-step size; render loop may skip or interpolate
- [ ] **P1-G:** Simulation clock — `feature_roadmap.md §P1-G`
  - [ ] `SimClock` stores current time as Julian Date (`double`)
  - [ ] `setDate(JulianDate)` for arbitrary date jump (time travel hook)
  - [ ] `advance(dt_years)` for integration step
  - [ ] Utility: `julianDateFromCalendar(year, month, day, hour, min, sec) → double`
  - [ ] Default start: current wall-clock date
- [ ] **P1-H:** Two-body stable orbit demo — `feature_roadmap.md §P1-H`
  - [ ] Hand-tuned Sun + Earth setup (not from ephemeris yet)
  - [ ] Earth orbits stably for 100+ simulated years without spiral-in or escape
  - [ ] Camera orbits the scene; starfield visible; sphere shading looks correct
  - [ ] Verify: orbit radius remains within ±5% of initial value after 100 simulated years

### Deliverable
*A moon orbits a planet stably. Camera can orbit and zoom. Nothing else.*

---

## Phase 2: Inner Solar System

**Entry Criteria:** Phase 1 complete. Integrator stable, entity system working.
**Exit Criteria:** Sun, Mercury, Venus, Earth (+ Moon), Mars positioned from DE440 ephemeris data. Time warp works. Orbit trails render. Numerical validation passes for Earth.

### Checklist

- [ ] **P2-A:** JPL ephemeris integration — `feature_roadmap.md §P2-A`
  - [ ] `jpl_eph` C library integrated via CMake FetchContent or `extern/jpl_eph/` submodule
  - [ ] `EphemerisProvider` C++ wrapper: `getStateVector(bodyId, julianDate) → {position, velocity}`
  - [ ] Body IDs use JPL's standard codes (10 = Sun, 399 = Earth, etc.)
  - [ ] DE440 file path configurable via CMake cache variable defaulting to `data/ephemeris/de440.bsp`
  - [ ] Verify: `EphemerisProvider` returns Earth's position at 2000-Jan-1.5 (J2000); check against known value
- [ ] **P2-B:** Inner planet bodies — `feature_roadmap.md §P2-B`
  - [ ] Data for Sun, Mercury, Venus, Earth, Mars: mass (solar masses), physical radius (km), color placeholder
  - [ ] State vectors initialized from DE440 at `SimClock` start date
  - [ ] Verify: all 5 bodies render and orbit (even at wrong scale — scale system comes in P2-D)
- [ ] **P2-C:** Luna — `feature_roadmap.md §P2-C`
  - [ ] Earth's Moon added as Tier 1 simulated body (JPL ID 301)
  - [ ] State vector from DE440
  - [ ] Visually distinguishable from Earth (smaller, greyer)
- [ ] **P2-D:** Scale system — `feature_roadmap.md §P2-D`
  - [ ] `ScaleConfig`: `sizeScale` (radius multiplier), `distanceScale` (position multiplier)
  - [ ] Applied in render transform, not in physics (physics always in real AU)
  - [ ] Presets: "Display" (sizeScale ~1000×, distanceScale ~0.3×), "True Scale" (both 1×)
  - [ ] Toggle key (e.g., T) cycles between presets
- [ ] **P2-E:** Time warp — `feature_roadmap.md §P2-E`
  - [ ] Discrete warp levels: ±1×, ±10×, ±100×, ±1,000×, ±10,000×, ±100,000×
  - [ ] Pause (0×)
  - [ ] UI: keyboard shortcuts (`,`/`.` to step down/up; `Space` to pause)
  - [ ] Default warp tuned: Earth orbits in ~2 minutes at default warp
- [ ] **P2-F:** Orbit trails — `feature_roadmap.md §P2-F`
  - [ ] Ring buffer of N recent positions per body (N = 500 default)
  - [ ] Drawn as `GL_LINE_STRIP` with per-vertex alpha falloff (oldest = transparent)
  - [ ] Trail length configurable. Toggle key (e.g., O)
- [ ] **P2-G:** Basic Sun glow — `feature_roadmap.md §P2-G`
  - [ ] Additive billboard quad around the Sun, scaled with distance
  - [ ] Simple alpha falloff from center; orange-white tint
  - [ ] Replaced by full HDR bloom in Phase 4 (P4-G)
- [ ] **P2-H:** Numerical validation — inner planets — `feature_roadmap.md §P2-H`
  - [ ] Generate Earth position at 3 dates from JPL Horizons (J2000, 2010-Jan-1, 2020-Jun-21)
  - [ ] Compare against `EphemerisProvider` output. Error < 0.01 AU (< 1% of Earth's orbit)
  - [ ] Document results in `tests/validation/inner_planets.md`

### Deliverable
*Inner solar system with stable, visually distinct orbits. Time warp works. Scale feels right.*

---

## Phase 3: Full Solar System

**Entry Criteria:** Phase 2 complete. Ephemeris pipeline working. Inner planets validated.
**Exit Criteria:** All planets, all dwarf planets, all Tier 1–3 moons. Saturn's rings visible. Asteroid/Kuiper belts present. Basic autopilot runs.

### Checklist

- [ ] **P3-A:** Outer planet bodies — `feature_roadmap.md §P3-A`
  - [ ] Jupiter (599), Saturn (699), Uranus (799), Neptune (899) added
  - [ ] State vectors from DE440
  - [ ] Color placeholder (gas giant colors: cream, tan, cyan, blue)
- [ ] **P3-B:** Dwarf planet bodies — `feature_roadmap.md §P3-B`
  - [ ] Pluto (999), Ceres (Ceres DE440 ID), Eris, Haumea, Makemake
  - [ ] Keplerian fallback for bodies outside DE440's primary target set
  - [ ] `KeplerianBody` implementation using classical orbital elements → position function
- [ ] **P3-C:** Tier 1 moons — `feature_roadmap.md §P3-C`
  - [ ] Io (501), Europa (502), Ganymede (503), Callisto (504)
  - [ ] Titan (606), Rhea (605)
  - [ ] Triton (801)
  - [ ] Charon (901)
  - [ ] Titania (703), Oberon (704)
  - [ ] All state vectors from DE440. All gravitationally simulated.
  - [ ] Verify: Galilean moons orbit Jupiter correctly (Io ~1.77 day period)
- [ ] **P3-D:** Tier 2 moons — `feature_roadmap.md §P3-D`
  - [ ] ~50–70 named moons as `DecorativeBody` using Keplerian path functions
  - [ ] Include: Phobos, Deimos, Enceladus, Mimas, Miranda, Hyperion, Nereid, Puck, Proteus (representative set)
  - [ ] Clickable for info in Phase 5 (structure supports it now; UI comes later)
- [ ] **P3-E:** Tier 3 moon swarms — `feature_roadmap.md §P3-E`
  - [ ] Instanced sphere rendering: one draw call per planet for swarm
  - [ ] Per-instance data: position (CPU-updated), radius (fixed small), base color
  - [ ] Jupiter: ~150 particles in 2 orbital bands; Saturn: ~100; Uranus/Neptune: ~50 each
  - [ ] Positions randomized within correct orbital zone at startup, advance at approximately correct rates
- [ ] **P3-F:** Saturn's rings — `feature_roadmap.md §P3-F`
  - [ ] Flat annular mesh (textured) around Saturn's equatorial plane
  - [ ] Ring texture: grayscale density/opacity map (D, C, B, A, F, G, E ring zones)
  - [ ] Alpha-blended, renders front and back
  - [ ] Slightly inclined to match Saturn's actual axial tilt (~26.7°)
- [ ] **P3-G:** Uranus & Neptune rings — `feature_roadmap.md §P3-G`
  - [ ] Same approach as P3-F, thinner and more transparent
  - [ ] Uranus: 13 known rings; Neptune: 5 rings (Adams, Le Verrier, Arago, Lassell, Galle)
- [ ] **P3-H:** Asteroid belt — `feature_roadmap.md §P3-H`
  - [ ] Instanced point/small-sphere rendering, 3,000–5,000 particles
  - [ ] Elliptical band 2.2–3.2 AU from Sun, slight inclination scatter
  - [ ] Low-opacity to not overpower the planets
- [ ] **P3-I:** Kuiper belt — `feature_roadmap.md §P3-I`
  - [ ] Same approach as P3-H, 1,000–2,000 particles, 30–55 AU band
  - [ ] Even lower opacity — the Kuiper belt is extremely sparse
- [ ] **P3-J:** Basic cinematic autopilot — `feature_roadmap.md §P3-J`
  - [ ] Camera visits each planet in sequence, orbiting for 5–10 seconds before moving to the next
  - [ ] Smooth eased transitions between positions
  - [ ] Activated by toggling autopilot mode (e.g., A key)
  - [ ] No authored vantage points yet — those come in P5-A

### Deliverable
*The complete solar system is present. Gas giant systems feel alive with moon swarms. Belts and rings provide visual context. You can leave it running on a screen and it looks beautiful.*

---

## Phase 4: Visual Polish

**Entry Criteria:** Phase 3 complete. Full solar system present and running.
**Exit Criteria:** Real NASA textures on all bodies. Atmospheric scattering on Earth/Venus/Titan. Shadow mapping for eclipses. HDR + bloom. Real star catalog background.

### Checklist

- [ ] **P4-A:** Planet texture mapping — `feature_roadmap.md §P4-A`
  - [ ] `stb_image` integration (header already in `extern/stb/`)
  - [ ] `Texture2D` class: load PNG/JPG, generate mipmaps, bind to shader
  - [ ] Equirectangular UV mapping on all spheres
  - [ ] Texture pack downloaded from Solar System Scope (document source in README)
  - [ ] All 8 planets + Sun + dwarf planets + Tier 1 moons textured
- [ ] **P4-B:** Texture LOD — `feature_roadmap.md §P4-B`
  - [ ] Two texture sizes per body: `<body>_4k.jpg` (close), `<body>_1k.jpg` (distant)
  - [ ] Distance threshold per body type; swap on camera distance change
  - [ ] OpenGL mipmap generation handles in-between sizes
- [ ] **P4-C:** Earth special layers — `feature_roadmap.md §P4-C`
  - [ ] Clouds: separate texture blended additively, slightly offset UV animated slowly
  - [ ] Night lights: emissive texture, multiplied by (1 - sunDot) to show only on dark side
  - [ ] Ocean specular: specular mask texture; land has low shininess, ocean has high
- [ ] **P4-D:** Gas giant cloud animation — `feature_roadmap.md §P4-D`
  - [ ] UV scroll offset on cloud bands: different bands move at slightly different rates
  - [ ] Very slow (barely perceptible in real-time; visible at ×10,000 warp)
- [ ] **P4-E:** Hipparcos star catalog — `feature_roadmap.md §P4-E`
  - [ ] Load JSON star catalog (~118,000 stars, filtered to magnitude < 7.0)
  - [ ] Convert RA/Dec to 3D unit vectors on the celestial sphere
  - [ ] Render as GL_POINTS with magnitude-driven size/brightness
  - [ ] Replaces procedural starfield from P1-C
- [ ] **P4-F:** HDR rendering pipeline — `feature_roadmap.md §P4-F`
  - [ ] Framebuffer with `GL_RGBA16F` color attachment
  - [ ] Post-process pass: tone mapping (Reinhard or ACES filmic)
  - [ ] Foundation for bloom (P4-G), atmosphere (P4-H), god rays (P4-J)
- [ ] **P4-G:** Sun bloom — `feature_roadmap.md §P4-G`
  - [ ] Bright-pass filter: extract regions above luminance threshold
  - [ ] Progressive downsampling/upsampling Gaussian blur (LearnOpenGL physically-based bloom)
  - [ ] Composite bloom back onto HDR framebuffer before tone mapping
  - [ ] Replaces basic Sun glow billboard from P2-G
- [ ] **P4-H:** Atmospheric scattering — `feature_roadmap.md §P4-H`
  - [ ] Rayleigh + Mie scattering fragment shader (adapted from `wwwtyro/glsl-atmosphere`)
  - [ ] Applied to Earth (blue), Venus (thick orange-white), Titan (hazy orange)
  - [ ] Atmosphere parameters: planet radius, atmosphere thickness, scattering coefficients per body
  - [ ] Visible light wrap around planet limb from space-view
- [ ] **P4-I:** Terminator line — `feature_roadmap.md §P4-I`
  - [ ] Fragment shader: `smoothstep` on `dot(normal, sunDir)` in penumbra zone
  - [ ] Applied to all textured bodies (not just Earth)
  - [ ] Width and softness tunable per body type
- [ ] **P4-J:** God rays — `feature_roadmap.md §P4-J`
  - [ ] Screen-space radial blur from projected Sun position
  - [ ] Active only when Sun is behind or near the limb of a body
  - [ ] Intensity falls off with angular distance from occlusion
- [ ] **P4-K:** Shadow mapping — `feature_roadmap.md §P4-K`
  - [ ] Cubemap shadow map rendered from Sun's position each frame
  - [ ] Depth test in main fragment shader to determine shadow
  - [ ] PCF (Percentage Closer Filtering) for soft shadow edges
  - [ ] Verify: basic shadow from Earth visible on Moon and vice versa
- [ ] **P4-L:** Solar eclipses — `feature_roadmap.md §P4-L`
  - [ ] Moon's shadow cone visible on Earth's surface during alignment
  - [ ] Uses shadow map from P4-K; no additional geometry
  - [ ] Verify: position simulation clock to a known solar eclipse date; shadow appears correctly
- [ ] **P4-M:** Lunar eclipses — `feature_roadmap.md §P4-M`
  - [ ] Earth's shadow on the Moon; Moon takes on reddish tint (Rayleigh-scattered limb light)
  - [ ] Red tint applied in fragment shader when Moon is in Earth's shadow cone
- [ ] **P4-N:** Jupiter moon shadow dots — `feature_roadmap.md §P4-N`
  - [ ] Galilean moon shadow dots appear on Jupiter's cloud bands
  - [ ] Uses same shadow map system as P4-K
- [ ] **P4-O:** Lens flare *(optional toggle)* — `feature_roadmap.md §P4-O`
  - [ ] Screen-space lens flare when Sun is in frame
  - [ ] Off by default. Toggle in settings/keybinding

### Deliverable
*Someone walks past the screen and stops to stare. The lighting is beautiful. An eclipse happens and it's breathtaking.*

---

## Phase 5: Interaction & Time Travel

**Entry Criteria:** Phase 4 complete. Visual presentation is finished.
**Exit Criteria:** Full interactive camera system. Date picker. Curated event library. Time travel works and is validated numerically.

### Checklist

- [ ] **P5-A:** Curated cinematic autopilot — `feature_roadmap.md §P5-A`
  - [ ] Authored list of vantage points: body target, camera offset, duration, transition duration
  - [ ] Smooth eased camera interpolation between points (cubic Bézier or slerp)
  - [ ] At least 8 authored points at Phase 5 release (Saturn rings sunrise, Europa close pass, etc.)
  - [ ] Replaces basic autopilot from P3-J
- [ ] **P5-B:** Click-to-follow — `feature_roadmap.md §P5-B`
  - [ ] Ray cast from click position through camera frustum to body spheres
  - [ ] Camera smoothly transitions to tracking selected body
  - [ ] Deselect by clicking empty space or pressing ESC
- [ ] **P5-C:** Lock-to orbit mode — `feature_roadmap.md §P5-C`
  - [ ] Once a body is selected (P5-B), a secondary lock key (e.g., L) switches to orbit mode
  - [ ] Camera orbits the selected body; scroll zooms relative to the body
- [ ] **P5-D:** Free-fly camera — `feature_roadmap.md §P5-D`
  - [ ] WASD movement relative to camera facing
  - [ ] Mouse look (hold right-click to aim)
  - [ ] Speed scales logarithmically (slow near planets, fast in open space)
- [ ] **P5-E:** Smooth camera mode transitions — `feature_roadmap.md §P5-E`
  - [ ] State machine: AUTOPILOT → FOLLOW → ORBIT → FREE_FLY
  - [ ] Each transition eases over 1–2 seconds; no jump cuts
- [ ] **P5-F:** Body selection & info panel — `feature_roadmap.md §P5-F`
  - [ ] Info panel shows: name, type, mass (kg), radius (km), orbital period, current velocity (km/s), distance from Sun (AU and km)
  - [ ] Panel fades in on selection, hidden otherwise
  - [ ] Panel positioned to avoid overlapping the body's rendered position
- [ ] **P5-G:** Minimal UI framework — `feature_roadmap.md §P5-G`
  - [ ] All UI elements hidden in passive mode (no mouse movement for N seconds)
  - [ ] Fade in on mouse movement, fade out on inactivity
  - [ ] Art piece mode = full passive (override to suppress all UI)
- [ ] **P5-H:** Date picker — `feature_roadmap.md §P5-H`
  - [ ] UI element: text input for `YYYY-MM-DD` (or `YYYY-MM-DD HH:MM`)
  - [ ] Validates input is within DE440 range (1550–2650 at full accuracy; DE441 beyond)
  - [ ] On confirm: body positions re-evaluated from ephemeris at target Julian Date
  - [ ] SimClock jumps to target date; integration continues from there
- [ ] **P5-I:** Playback speed dial — `feature_roadmap.md §P5-I`
  - [ ] Continuous drag control (UI slider or scroll wheel on speed display)
  - [ ] Range: −centuries/sec to +centuries/sec
  - [ ] Visual indicator of current rate and direction
- [ ] **P5-J:** Ephemeris confidence indicator — `feature_roadmap.md §P5-J`
  - [ ] Date display color: green (1550–2650 DE440 full accuracy), yellow (outside range, DE441), orange (projected, speculative)
  - [ ] Tooltip on hover explaining the distinction
- [ ] **P5-K:** Date/time overlay — `feature_roadmap.md §P5-K`
  - [ ] Current simulation date always visible (small corner text)
  - [ ] Format: `2024-04-08` or `April 8, 2024` (configurable)
  - [ ] Fades slightly but never fully hidden (even in art piece mode, date persists at low opacity)
- [ ] **P5-L:** Curated event library — `feature_roadmap.md §P5-L`
  - [ ] Data-driven: list of events in a JSON or C++ data file (date, camera setup, description)
  - [ ] UI: scrollable list, grouped by category (Historic, Eclipses, Milestones)
  - [ ] Selecting an event: jump date + fly camera to authored position
  - [ ] Initial set: ≥10 events across all three categories
- [ ] **P5-M:** Eclipse frequency tuning — `feature_roadmap.md §P5-M`
  - [ ] "Display Mode" orbital inclinations: subtle adjustments so eclipses occur ~12× more often
  - [ ] "True Accuracy Mode" toggle restores real inclinations
  - [ ] Setting persists across sessions
- [ ] **P5-N:** Numerical validation — event dates — `feature_roadmap.md §P5-N`
  - [ ] Compare computed positions at 3+ curated event dates against JPL Horizons
  - [ ] Document results in `tests/validation/event_dates.md`
  - [ ] Gate: validation must pass before Phase 5 is considered complete

### Deliverable
*You can leave it running in autopilot mode all day, or sit down and explore the solar system on the date of the Moon landing with the camera framing the Earth-Moon system.*

---

## Phase 6: Sandbox & Extras

**Entry Criteria:** Phase 5 complete. Interaction and time travel fully working.
**Exit Criteria:** Custom body spawning, collision, spacecraft models. The simulation is a toy, not just a screensaver.

### Checklist

- [ ] **P6-A:** Custom body spawning — `feature_roadmap.md §P6-A`
  - [ ] UI: mass slider, radius input, color picker
  - [ ] Click in the simulation view to place; drag to set initial velocity vector
  - [ ] Body enters N-body simulation on confirm
  - [ ] "Remove all custom bodies" reset button
- [ ] **P6-B:** Collision detection & response — `feature_roadmap.md §P6-B`
  - [ ] Per-frame sphere-sphere overlap test for all simulated body pairs
  - [ ] Response: if relative velocity < threshold → merge (sum masses, momentum-conserving velocity); else → shatter (2–4 fragments)
  - [ ] Merge produces a new `SimulatedBody`; shatter produces short-lived decorative fragments
- [ ] **P6-C:** Chaos mode — `feature_roadmap.md §P6-C`
  - [ ] Button: "Chaos" — spawns N equal-mass stars (default N=16) distributed across inner system
  - [ ] Bodies interact gravitationally; collisions enabled
  - [ ] "Reset to Solar System" button restores original bodies
- [ ] **P6-D:** Comet bodies — `feature_roadmap.md §P6-D`
  - [ ] Pre-configured named comets: Halley, Hale-Bopp, Shoemaker-Levy 9 (pre-1994 path)
  - [ ] Visual tail: additive particle sprite trail pointing away from the Sun
  - [ ] Spawneable as custom body with comet-type preset (high eccentricity, elongated orbit)
- [ ] **P6-E:** Gravitational field visualization — `feature_roadmap.md §P6-E`
  - [ ] Toggleable overlay: warped grid or heat map showing gravitational potential
  - [ ] Grid: 2D plane through the ecliptic, deformed by sum of gravitational wells
  - [ ] Computed on CPU once per second (not every frame); uploaded as texture
- [ ] **P6-F:** Gravitational lensing effect — `feature_roadmap.md §P6-F`
  - [ ] Post-process shader: radial distortion around massive bodies
  - [ ] Effect magnitude proportional to body mass and inverse square distance from body
  - [ ] Subtle on real solar system; dramatic in chaos mode
- [ ] **P6-G:** Mini-map overlay — `feature_roadmap.md §P6-G`
  - [ ] Corner overlay: top-down 2D orthographic view of the ecliptic plane
  - [ ] Bodies shown as colored dots with name labels
  - [ ] Camera frustum shown as a triangle indicating viewing direction
  - [ ] Click a dot to select the body (same as P5-B)
  - [ ] Toggle key (e.g., M)
- [ ] **P6-H:** Spacecraft models — `feature_roadmap.md §P6-H`
  - [ ] Simple OBJ/GLTF loader for NASA 3D model assets (or impostor sprites if models are too complex)
  - [ ] Initial set: Hubble (LEO), JWST (L2), Voyager 1, Voyager 2
  - [ ] Trajectory: Hubble and JWST use Keplerian prescriptions; Voyagers from Horizons tables or SPICE
  - [ ] Clickable for info panel (P5-F)
- [ ] **P6-I:** SPICE trajectory data — `feature_roadmap.md §P6-I`
  - [ ] CSPICE toolkit integrated for Voyager 1 & 2 current positions
  - [ ] SPK kernels downloaded to `data/spice/` alongside ephemeris
  - [ ] `SpiceProvider` class mirrors `EphemerisProvider` interface
- [ ] **P6-J:** Personal date bookmarks *(stretch goal)* — `feature_roadmap.md §P6-J`
  - [ ] "Bookmark this date" button in date picker
  - [ ] Prompts for a label
  - [ ] Persisted to `~/.circa-solem/bookmarks.json` (or platform equivalent)
  - [ ] Appears in the event library under "My Bookmarks" category

### Deliverable
*You zoom out from Earth, spot JWST sitting at L2, then hurl a rogue Jupiter at the inner solar system and watch everything scatter.*

---

## Spec Index

Master index of all specification documents. Status: PENDING | DRAFT | REVIEW | FINAL

Spec files live in `specs/<category>/`.

### Physics (PHY)

| ID | Name | Status | Phase | Roadmap |
|----|------|--------|-------|---------|
| PHY-001 | Velocity Verlet Integrator | PENDING | 1 | P1-D |
| PHY-002 | N-Body Simulation | PENDING | 1 | P1-D, P1-E |
| PHY-003 | Unit System (AU / Solar Mass / Year) | PENDING | 1 | P1-D |
| PHY-004 | Keplerian Body Path Functions | PENDING | 3 | P3-B, P3-D |
| PHY-005 | Collision Detection & Response | PENDING | 6 | P6-B |

### Renderer (RND)

| ID | Name | Status | Phase | Roadmap |
|----|------|--------|-------|---------|
| RND-001 | Sphere Geometry & Phong Shader | PENDING | 1 | P1-A |
| RND-002 | Instanced Rendering Pipeline | PENDING | 3 | P3-E, P3-H, P3-I |
| RND-003 | HDR Framebuffer & Tone Mapping | PENDING | 4 | P4-F |
| RND-004 | Shadow Mapping (Cubemap / Point Light) | PENDING | 4 | P4-K |
| RND-005 | Atmospheric Scattering Shader | PENDING | 4 | P4-H |
| RND-006 | Bloom (Physically-Based) | PENDING | 4 | P4-G |
| RND-007 | God Rays (Screen-Space) | PENDING | 4 | P4-J |
| RND-008 | Texture LOD System | PENDING | 4 | P4-B |

### Camera (CAM)

| ID | Name | Status | Phase | Roadmap |
|----|------|--------|-------|---------|
| CAM-001 | Orbit Camera (Arcball) | PENDING | 1 | P1-B |
| CAM-002 | Basic Cinematic Autopilot | PENDING | 3 | P3-J |
| CAM-003 | Curated Cinematic Autopilot | PENDING | 5 | P5-A |
| CAM-004 | Click-to-Follow / Lock-to Orbit | PENDING | 5 | P5-B, P5-C |
| CAM-005 | Free-Fly Camera | PENDING | 5 | P5-D |
| CAM-006 | Camera Mode State Machine & Transitions | PENDING | 5 | P5-E |

### Ephemeris (EPH)

| ID | Name | Status | Phase | Roadmap |
|----|------|--------|-------|---------|
| EPH-001 | SimClock (Julian Date) | PENDING | 1 | P1-G |
| EPH-002 | jpl_eph Integration (DE440) | PENDING | 2 | P2-A |
| EPH-003 | EphemerisProvider Interface | PENDING | 2 | P2-A |
| EPH-004 | SpiceProvider Interface | PENDING | 6 | P6-I |

### Bodies (BDY)

| ID | Name | Status | Phase | Roadmap |
|----|------|--------|-------|---------|
| BDY-001 | Body Entity System (Simulated / Decorative) | PENDING | 1 | P1-E |
| BDY-002 | Inner Planet Data | PENDING | 2 | P2-B |
| BDY-003 | Luna (Tier 1 Moon) | PENDING | 2 | P2-C |
| BDY-004 | Outer Planet Data | PENDING | 3 | P3-A |
| BDY-005 | Dwarf Planet Data | PENDING | 3 | P3-B |
| BDY-006 | Tier 1 Moon Data | PENDING | 3 | P3-C |
| BDY-007 | Tier 2 Moon Data (Named Decorative) | PENDING | 3 | P3-D |
| BDY-008 | Tier 3 Moon Swarms | PENDING | 3 | P3-E |
| BDY-009 | Saturn Ring Geometry | PENDING | 3 | P3-F |
| BDY-010 | Asteroid / Kuiper Belt Particles | PENDING | 3 | P3-H, P3-I |
| BDY-011 | Spacecraft Decorative Bodies | PENDING | 6 | P6-H |

### Scale & Time (SCL / TIM)

| ID | Name | Status | Phase | Roadmap |
|----|------|--------|-------|---------|
| SCL-001 | Scale System (Size / Distance) | PENDING | 2 | P2-D |
| TIM-001 | Time Warp Controls | PENDING | 2 | P2-E |
| TIM-002 | Date Picker & Time Travel | PENDING | 5 | P5-H, P5-I |
| TIM-003 | Eclipse Frequency Tuning | PENDING | 5 | P5-M |

### UI (UI)

| ID | Name | Status | Phase | Roadmap |
|----|------|--------|-------|---------|
| UI-001 | Minimal UI Framework (Fade-on-Interaction) | PENDING | 5 | P5-G |
| UI-002 | Body Info Panel | PENDING | 5 | P5-F |
| UI-003 | Date/Time Overlay | PENDING | 5 | P5-K |
| UI-004 | Ephemeris Confidence Indicator | PENDING | 5 | P5-J |
| UI-005 | Curated Event Library | PENDING | 5 | P5-L |
| UI-006 | Mini-Map Overlay | PENDING | 6 | P6-G |
| UI-007 | Personal Bookmarks | PENDING | 6 | P6-J |

### Sandbox (SBX)

| ID | Name | Status | Phase | Roadmap |
|----|------|--------|-------|---------|
| SBX-001 | Custom Body Spawning | PENDING | 6 | P6-A |
| SBX-002 | Collision Detection & Response | PENDING | 6 | P6-B |
| SBX-003 | Chaos Mode | PENDING | 6 | P6-C |
| SBX-004 | Comet Bodies | PENDING | 6 | P6-D |
| SBX-005 | Gravitational Field Visualization | PENDING | 6 | P6-E |
| SBX-006 | Gravitational Lensing Effect | PENDING | 6 | P6-F |

### Validation (VAL)

| ID | Name | Status | Phase | Roadmap |
|----|------|--------|-------|---------|
| VAL-001 | Inner Planet Position Validation | PENDING | 2 | P2-H |
| VAL-002 | Event Date Position Validation | PENDING | 5 | P5-N |

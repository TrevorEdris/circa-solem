# Phase 1 — Orbit Stability Validation

**Date:** 2026-03-19
**Build:** phase/1-engine-foundation

## Setup

Two-body demo: Sun (1 M☉) + Earth (3.003×10⁻⁶ M☉) in circular orbit.
- Earth initial position: (1, 0, 0) AU
- Earth initial velocity: (0, 0, -2π) AU/yr — circular orbit velocity at 1 AU, XZ plane (Y-up convention)
- Warp factor: 1000× (Earth completes one orbit in ~9 minutes wall-clock time)

## Unit Tests (automated)

| Test | Result |
|------|--------|
| Energy conservation <0.1% over 1000 steps (dt=1/365 yr) | PASS |
| Earth completes ~1 orbit in 365 steps (cos_angle > 0.9994) | PASS |
| Decorative bodies not integrated | PASS |

## Visual Validation (manual)

To verify: `make run`, observe for 5+ minutes at 1000× warp.

Expected observations:
- Earth (blue sphere) orbits the Sun (yellow sphere) in a stable ellipse
- Orbit radius visually constant — no spiral inward or outward
- Starfield visible in background, stationary relative to camera motion
- Drag rotates camera, scroll zooms, ESC exits cleanly
- No stderr output on clean launch

## Notes

- Normal matrix computed per-frame as `transpose(inverse(mat3(model)))` — acceptable cost for ≤20 bodies
- Sun rendered at 0.05 AU radius, Earth at 0.02 AU (display scale for visibility — Phase 2 adds ScaleConfig)
- Far-plane depth trick in `starfield.vert` (`gl_Position = clip_pos.xyww`) confirmed working: stars appear behind all geometry at all zoom levels

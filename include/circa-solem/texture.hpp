#pragma once

#include <glad/gl.h>
#include <string>

namespace cs {

/// Load an image file as a GL_TEXTURE_2D with mipmaps.
/// Returns the texture ID, or 0 on failure (logs to stderr).
GLuint loadTexture(const std::string& path);

/// Generate a procedural Saturn-like ring density texture (256×1 RGBA).
/// Models the D, C, B, Cassini Division, A, and F ring zones.
GLuint generateSaturnRingTexture();

/// Generate a faint, thin ring texture for Uranus or Neptune (256×1 RGBA).
GLuint generateFaintRingTexture();

} // namespace cs

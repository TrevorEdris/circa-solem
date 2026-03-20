#include "circa-solem/texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cstdio>

namespace cs {

GLuint loadTexture(const std::string& path) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data) {
        fprintf(stderr, "loadTexture: failed to load '%s': %s\n",
                path.c_str(), stbi_failure_reason());
        return 0;
    }

    GLenum format = GL_RGB;
    if (channels == 1)      format = GL_RED;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 4) format = GL_RGBA;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format),
                 width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return tex;
}

// Helper: create a 1-pixel-tall RGBA texture from a pixel array.
static GLuint makeTexture1D(const unsigned char* pixels, int width) {
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, 1, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return tex;
}

GLuint generateSaturnRingTexture() {
    // 256-pixel radial profile: D ring, C ring, B ring, Cassini Division, A ring, F ring.
    // Pixel 0 = inner edge (66,900 km), pixel 255 = outer edge (136,775 km).
    // Total span: 69,875 km. Each pixel ≈ 273 km.
    constexpr int W = 256;
    unsigned char pixels[W * 4];

    auto set_pixel = [&](int i, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
        pixels[i * 4 + 0] = r;
        pixels[i * 4 + 1] = g;
        pixels[i * 4 + 2] = b;
        pixels[i * 4 + 3] = a;
    };

    // Approximate ring zone boundaries as pixel indices:
    // D ring:  0-16   (faint)
    // C ring:  16-59  (moderate)
    // B ring:  59-113 (bright, densest)
    // Cassini: 113-122 (gap)
    // A ring:  122-163 (bright)
    // Encke:   ~150    (thin gap)
    // F ring:  ~170    (narrow, faint)
    // Beyond:  170-255 (empty)

    for (int i = 0; i < W; ++i) {
        unsigned char brightness = 0;
        unsigned char alpha = 0;

        if (i < 16) {
            // D ring — very faint
            brightness = 60;
            alpha = 20;
        } else if (i < 59) {
            // C ring — moderate
            brightness = 140;
            alpha = 80;
        } else if (i < 113) {
            // B ring — brightest
            brightness = 200;
            alpha = 200;
        } else if (i < 122) {
            // Cassini Division — gap
            brightness = 30;
            alpha = 5;
        } else if (i < 163) {
            // A ring — bright
            brightness = 180;
            alpha = 160;
            // Encke gap
            if (i >= 149 && i <= 151) { brightness = 30; alpha = 5; }
        } else if (i >= 169 && i <= 172) {
            // F ring — narrow, faint
            brightness = 100;
            alpha = 40;
        }
        // else: empty (alpha = 0)

        set_pixel(i, brightness, brightness, brightness, alpha);
    }

    return makeTexture1D(pixels, W);
}

GLuint generateFaintRingTexture() {
    // Thin, faint ring profile for Uranus/Neptune.
    constexpr int W = 64;
    unsigned char pixels[W * 4];

    for (int i = 0; i < W; ++i) {
        unsigned char brightness = 80;
        unsigned char alpha = 0;

        // A few thin ring bands scattered across the radial range
        if ((i >= 8 && i <= 10) || (i >= 20 && i <= 22) ||
            (i >= 35 && i <= 38) || (i >= 50 && i <= 55)) {
            alpha = 30 + static_cast<unsigned char>((i % 7) * 5);
        }

        pixels[i * 4 + 0] = brightness;
        pixels[i * 4 + 1] = brightness;
        pixels[i * 4 + 2] = brightness;
        pixels[i * 4 + 3] = alpha;
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, 1, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return tex;
}

} // namespace cs

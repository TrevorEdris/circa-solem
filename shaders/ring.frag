#version 410 core

in vec2 vUV;

uniform sampler2D ring_texture;
uniform vec3      tint_color;

out vec4 fragColor;

void main() {
    // Sample ring opacity from texture — UV.x maps radial distance (inner→outer)
    vec4 tex = texture(ring_texture, vUV);

    // Use texture luminance as alpha (grayscale ring density maps)
    float alpha = tex.a;
    // For RGB-only textures, derive alpha from luminance
    if (alpha > 0.99) {
        alpha = dot(tex.rgb, vec3(0.299, 0.587, 0.114));
    }

    // Discard fully transparent fragments
    if (alpha < 0.01) discard;

    fragColor = vec4(tint_color * tex.rgb, alpha * 0.8);
}

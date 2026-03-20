#version 410 core

in float brightness;

uniform vec3 particle_color;

out vec4 frag_color;

void main() {
    // Circular point shape
    vec2 coord = gl_PointCoord - vec2(0.5);
    if (dot(coord, coord) > 0.25) discard;

    frag_color = vec4(particle_color * brightness, brightness * 0.6);
}

#version 410 core

in float brightness;

out vec4 frag_color;

void main() {
    // Circular point shape: discard corners of the point sprite quad.
    vec2 coord = gl_PointCoord - vec2(0.5);
    if (dot(coord, coord) > 0.25) discard;

    frag_color = vec4(1.0, 1.0, 1.0, brightness);
}

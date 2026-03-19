#version 410 core

out vec4 fragColor;

void main() {
    // Solid magenta: visible proof-of-life that the shader pipeline is working
    fragColor = vec4(1.0, 0.0, 1.0, 1.0);
}

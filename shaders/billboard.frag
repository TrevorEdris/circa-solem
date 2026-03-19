#version 410 core

in  vec2 vUV;
out vec4 fragColor;

uniform vec3 uColor;

void main() {
    float dist  = length(vUV - vec2(0.5));
    float alpha = max(0.0, 1.0 - dist * 2.5);
    alpha       = pow(alpha, 2.0);  // soften falloff toward edges
    fragColor   = vec4(uColor * alpha, alpha);
}

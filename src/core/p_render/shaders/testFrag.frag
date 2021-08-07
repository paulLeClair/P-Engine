#version 450

layout (location = 0) out vec4 outFragColor;

void main() {
    // idk we'll just render a simple single-colored triangle
    outFragColor = vec4(1.0f, 0.5f, 0.0f, 1.0f);
}
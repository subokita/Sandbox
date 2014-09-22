#version 410 core

in vec2 uv;
out vec3 color;

uniform sampler2D texture_sampler;

void main() {
    color = texture( texture_sampler, uv ).xyz;
}
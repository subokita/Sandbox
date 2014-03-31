#version 410 core

layout( location = 0 ) in vec3 vertex_pos_m;
layout( location = 1 ) in vec2 uv_pos_m;

uniform mat4 ortho;

out vec2 uv;

void main() {
    gl_Position = ortho * vec4( vertex_pos_m, 1 );
    uv = uv_pos_m;
}
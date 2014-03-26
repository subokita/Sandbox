#version 410 core

layout( location = 0 ) in vec3 vertex_pos_m;

uniform mat4 mvp;

void main() {
    gl_Position = mvp * vec4( vertex_pos_m, 1 );
}
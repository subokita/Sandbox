#version 410 core

layout( location = 0 ) in vec3 vertex_pos_m;
layout( location = 1 ) in vec3 vertex_normals_m;

out vec3 position_w;
out vec3 normal_c;
out vec3 eye_direction_c;
out vec3 light_direction_c;

uniform vec3 light_position_w;
uniform mat4 mv;
uniform mat4 mvp;

void main() {
    gl_Position = mvp * vec4( vertex_pos_m, 1 );
    
    /* Vertex position in the world coord */
    position_w = (mv * vec4( vertex_pos_m, 1 )).xyz;
    
    /* Direction of eye */
    vec3 vertex_pos_c = (mv * vec4( vertex_pos_m, 1 ) ).xyz;
    eye_direction_c = vec3(0, 0, 0) - vertex_pos_c;
    
    /* Position of light */
    vec3 light_position_c = (mv * vec4(light_position_w, 1) ).xyz;
    light_direction_c = light_position_c + eye_direction_c;
    
    /* Normals in camera space */
    normal_c = (mv * vec4( vertex_normals_m, 0.0 )).xyz;
}
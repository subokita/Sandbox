#version 410 core

in vec3 normal_c;
in vec3 position_w;
in vec3 light_direction_c;
in vec3 eye_direction_c;

uniform vec3 light_position_w;

out vec4 frag_color;

void main() {
    float dist = length( light_position_w - position_w );
    
    vec3 n = normalize( normal_c );
    vec3 l = normalize( light_direction_c );
    float cos_diffuse = clamp( dot( n, l ), 0.0, 1.0 );
    
    vec3 e = normalize( eye_direction_c );
    vec3 r = reflect( -l, n );
    float cos_specular = clamp( dot( e, r ), 0.0, 1.0 );
    
    vec4 light_ambient      = vec4(0.3, 0.3, 0.3, 1.0);
    vec4 light_diffuse      = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 light_specular     = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 material_ambient   = vec4(0.0, 0.0, 1.0, 1.0);
    vec4 material_diffuse   = vec4(0.0, 0.0, 1.0, 1.0);
    vec4 material_specular  = vec4(1.0, 1.0, 1.0, 1.0);
    float material_shininess = 20.0f;
    
    vec4 color = light_ambient * material_ambient;
    if( cos_diffuse > 0.0 ){
        color += light_diffuse * material_diffuse * cos_diffuse;
        
        if( cos_specular > 0.0 )
            color += light_specular * material_specular * pow( cos_specular, material_shininess );
    }
    
    
    frag_color = color;
}

#version 330 core

in vec3 frag_pos;
in vec3 normal;

in vec2 uv;

uniform mat4 u_model_trans;
uniform mat4 u_view_trans;

uniform sampler2D depth_map;

out vec4 out_color;

void main()
{ 
    float depth = texture(depth_map, uv).r;
    out_color = vec4(vec3(depth), 1.0);
    //out_color = vec4(vec3(uv, 0.0), 1.0);
}
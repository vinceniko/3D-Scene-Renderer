#version 330 core

in vec3 a_pos;

out vec3 frag_pos;

uniform mat4 u_projection;
uniform mat4 u_view_trans;
uniform mat4 u_model_trans;

void main()
{
    frag_pos = vec3(u_model_trans * vec4(a_pos, 1.0));
    vec4 pos = u_projection * u_view_trans * vec4(frag_pos, 1.0);

    gl_Position = pos.xyzz; 
}
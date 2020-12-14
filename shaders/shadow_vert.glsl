#version 330 core

layout (location=0) in vec3 a_pos;

uniform mat4 u_light_vp;
uniform mat4 u_model_trans;

void main()
{
    gl_Position = u_light_vp * u_model_trans * vec4(a_pos, 1.0);
}
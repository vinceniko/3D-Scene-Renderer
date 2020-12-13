#version 330 core

in vec3 a_pos;

uniform mat4 u_light_view;
uniform mat4 u_model_trans;

void main()
{
    gl_Position = u_light_view * u_model_trans * vec4(a_pos, 1.0);
}
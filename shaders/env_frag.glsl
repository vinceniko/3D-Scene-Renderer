#version 330 core

in vec3 frag_pos;

uniform samplerCube u_skybox;

out vec4 out_color;

void main()
{             
    out_color = texture(u_skybox, frag_pos);
}
#version 330 core

in vec3 frag_pos;

uniform samplerCube cubemap;

out vec4 out_color;

void main()
{             
    out_color = texture(cubemap, frag_pos);
}  
#version 330 core
            
uniform vec3 triangle_color;

out vec4 out_color;

void main()
{
    out_color = vec4(triangle_color, 1.0);
}
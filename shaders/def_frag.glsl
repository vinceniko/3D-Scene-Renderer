#version 330 core
            
uniform vec3 u_object_color;

out vec4 out_color;

void main()
{
    out_color = vec4(u_object_color, 1.0);
}
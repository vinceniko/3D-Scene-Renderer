#version 330 core
            
uniform vec3 u_object_color;

out vec4 out_color;

void main()
{
    out_color = vec4(vec3(0.9, 0.7, 0.0), 1.0);
}
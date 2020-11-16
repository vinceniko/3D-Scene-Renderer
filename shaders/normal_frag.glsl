#version 330 core
            
uniform vec3 object_color;

out vec4 out_color;

void main()
{
    out_color = vec4(object_color, 1.0);
}
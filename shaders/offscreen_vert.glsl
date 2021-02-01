#version 330 core

layout (location=0) in vec3 a_pos;

out vec2 uv;

void main()
{   
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    float u = (x+1.0)*0.5;
    float v = (y+1.0)*0.5;

    uv = vec2(u, v);
    gl_Position = vec4(x, y, 0.0, 1.0); 
}
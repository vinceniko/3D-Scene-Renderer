#version 330 core

layout (location=0) in vec3 a_pos;

out vec2 uv;

void main()
{   
    float x = float((uint(gl_VertexID) << 1u) & uint(2)) / uint(2); 
    float y = float(uint(gl_VertexID) & uint(2)) / uint(2); 

    uv = vec2(x, y);
    gl_Position = vec4(uv * 2.0f + -1.0f, 0.0f, 1.0f);; 
}
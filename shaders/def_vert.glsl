#version 330 core

in vec3 a_pos;
in vec3 a_normal;

out vec3 normal;
out vec3 frag_pos;

out vec2 uv;

uniform mat4 u_projection;
uniform mat4 u_view_trans;
uniform mat4 u_model_trans;

void main()
{
    frag_pos = vec3(u_model_trans * vec4(a_pos, 1.0));
    normal = mat3(transpose(inverse(u_model_trans))) * a_normal;
    
    float x = float(((uint(gl_VertexID) + 2u) / 3u)%2u); 
    float y = float(((uint(gl_VertexID) + 1u) / 3u)%2u); 

    uv = vec2(x, y);
    gl_Position = u_projection * u_view_trans * vec4(frag_pos, 1.0); 
}
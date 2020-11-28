#version 330 core

in vec3 a_pos;
in vec3 a_normal;

out vec3 normal;
out vec3 frag_pos;

uniform mat4 projection;
uniform mat4 view_trans;
uniform mat4 model_trans;

void main()
{
    frag_pos = vec3(model_trans * vec4(a_pos, 1.0));
    normal = mat3(transpose(inverse(model_trans))) * a_normal;

    gl_Position = projection * view_trans * vec4(frag_pos, 1.0); 
}
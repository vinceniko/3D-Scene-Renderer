#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 normal;
out vec3 frag_pos;

uniform mat4 projection;
uniform mat4 view_trans;
uniform mat4 model_trans;

void main()
{
    frag_pos = vec3(model_trans * vec4(aPos, 1.0));
    normal = mat3(transpose(inverse(model_trans))) * aNormal;

    gl_Position = projection * view_trans * vec4(frag_pos, 1.0); 
}
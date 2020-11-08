#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT {
    vec3 normal;
} vs_out;

uniform mat4 projection;
uniform mat4 view_trans;
uniform mat4 model_trans;

void main()
{
    projection;
    gl_Position = view_trans * model_trans * vec4(aPos, 1.0); 
    mat3 normalMatrix = mat3(transpose(inverse(view_trans * model_trans)));
    vs_out.normal = normalize(vec3(vec4(normalMatrix * aNormal, 0.0)));
}
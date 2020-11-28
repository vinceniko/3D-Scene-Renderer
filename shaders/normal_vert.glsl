#version 330 core
in vec3 a_pos;
in vec3 a_normal;

out VS_OUT {
    vec3 normal;
} vs_out;

uniform mat4 u_projection;
uniform mat4 u_view_trans;
uniform mat4 u_model_trans;

void main()
{
    u_projection;
    gl_Position = u_view_trans * u_model_trans * vec4(a_pos, 1.0); 
    mat3 normalMatrix = mat3(transpose(inverse(u_view_trans * u_model_trans)));
    vs_out.normal = normalize(vec3(vec4(normalMatrix * a_normal, 0.0)));
}
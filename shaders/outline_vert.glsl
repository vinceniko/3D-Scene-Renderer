#version 330 core

layout (location=0) in vec3 a_pos;
layout (location=1) in vec3 a_normal;

out vec3 normal;
out vec3 frag_pos;

uniform mat4 u_projection;
uniform mat4 u_view_trans;
uniform mat4 u_model_trans;
uniform float u_aspect;

uniform mat4 u_light_vp;

void main()
{
    normal = mat3(transpose(inverse(u_model_trans))) * a_normal;
    frag_pos = vec3(u_model_trans * vec4(a_pos, 1.0));
    vec4 clip_pos = u_projection * u_view_trans * vec4(frag_pos, 1.0);
    //clip_pos /= clip_pos.w;
    vec4 clip_normal = u_projection * u_view_trans * vec4(normal, 0.0);

    clip_pos.xy += normalize(clip_normal.xy) * vec2(1 / u_aspect, 1.0) * 0.01 * clip_pos.w * 2;

    gl_Position = clip_pos; 
}
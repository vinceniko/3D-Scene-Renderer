#version 330 core
            
in vec3 frag_pos;
in vec3 normal;

uniform vec3 u_object_color;
uniform samplerCube u_skybox;
uniform mat4 u_view_trans;

out vec4 out_color;

void main()
{
    vec3 camera_pos = vec3(inverse(u_view_trans)[3]);
    vec3 eye_dir = normalize(frag_pos - camera_pos);
    vec3 reflected = reflect(eye_dir, normalize(normal));

    vec3 env_color = texture(u_skybox, reflected).rgb;

    out_color = vec4(mix(env_color, u_object_color, 0.05), 1.0);
}
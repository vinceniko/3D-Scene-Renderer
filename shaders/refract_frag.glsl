#version 330 core
            
in vec3 frag_pos;
in vec3 normal;

uniform vec3 u_object_color;
uniform samplerCube u_skybox;
uniform mat4 u_view_trans;

out vec4 out_color;

const float refractive_index = 1.52;

void main()
{
    float ratio = 1.00 / refractive_index;

    vec3 camera_pos = vec3(inverse(u_view_trans)[3]);
    vec3 eye_dir = normalize(frag_pos - camera_pos);
    vec3 refracted = refract(eye_dir, normalize(normal), ratio);

    vec3 env_color = texture(u_skybox, refracted).rgb;

    out_color = vec4(mix(env_color, u_object_color, 0.0), 1.0);
}
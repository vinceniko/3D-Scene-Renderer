#version 330 core
in vec3 frag_pos;
in vec3 normal;

uniform vec3 triangle_color;

uniform mat4 model_trans;
uniform mat4 view_trans;

out vec4 out_color;

void main()
{
    vec3 light_pos = vec3(0.0, 0.0, 20.0);
    vec3 lightColor = vec3(1.0);

    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light_pos - frag_pos);  
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse) * triangle_color;
    out_color = vec4(result, 1.0);
}
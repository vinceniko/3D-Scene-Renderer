#version 330 core

in vec3 frag_pos;
in vec3 normal;

uniform vec3 u_object_color;

uniform mat4 u_model_trans;
uniform mat4 u_view_trans;

in vec4 frag_pos_light;
uniform sampler2D shadow_map;

out vec4 out_color;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadow_map, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform DirLight dir_light;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 view_dir) {
    vec3 ambient = light.ambient;

    float diff = max(dot(normal, light.direction), 0.0);
    vec3 diffuse = light.diffuse * diff;

    // // phong
    // vec3 reflect_dir = reflect(-light.direction, normal);
    // float spec = pow(max(dot(view_dir, reflect_dir), 0.0), light.shininess);
    
    // blinn-phong
    vec3 half_dir = normalize(light.direction + view_dir);
    float spec = pow(max(dot(half_dir, normal), 0.0), light.shininess);
    vec3 specular = light.specular * spec;
        
    //vec3 result = max(vec3(1.0 - shadow), vec3(1, 0, 0)) * (ambient + (1.0 - shadow) * (diffuse + specular)) * u_object_color;
    vec3 result = (ambient + diffuse + specular) * u_object_color;
    //vec3 result = vec3(shadow, 0.0, 0.0);

    return result;
}

struct PointLight {   
    vec3 position;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;

    float constant;
    float linear;
    float quadratic;  
};  
#define NR_POINT_LIGHTS 1
uniform PointLight point_lights[NR_POINT_LIGHTS];

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_dir)
{
    vec3 light_dir = normalize(light.position - frag_pos);
    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    // specular shading
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), light.shininess);
    // attenuation
    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient  = light.ambient;
    vec3 diffuse  = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    
    vec3 result = (ambient + diffuse + specular) * u_object_color;
    
    return result;
}

void main()
{
    vec3 view_dir = normalize(vec3(inverse(u_view_trans)[3]) - frag_pos);
    vec3 norm = normalize(normal);

    vec3 lighting = CalcDirLight(dir_light, norm, view_dir);
    // point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        lighting += CalcPointLight(point_lights[i], norm, frag_pos, view_dir);    
    
    float shadow = ShadowCalculation(frag_pos_light);
    vec3 result;
    if (shadow == 1.0) {
        //result = vec3(shadow, 0.0, 0.0);
        result = 1 - vec3(shadow);
    } else {
        result = lighting * u_object_color;
    }
    
    out_color = vec4(result, 1.0);
}
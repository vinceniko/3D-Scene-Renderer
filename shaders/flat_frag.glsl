#version 330 core
in vec3 frag_pos;
in vec3 normal;

uniform vec3 u_object_color;

uniform mat4 u_model_trans;
uniform mat4 u_view_trans;

in vec4 frag_pos_light;
uniform sampler2D u_shadow_map;
uniform uint u_debug_shadows;

out vec4 out_color;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(u_shadow_map, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float base_bias = 0.03;
    float dir_light = (1.0 - dot(normal, vec3(fragPosLightSpace)));
    float bias = max(base_bias * dir_light, base_bias);
    float shadow = 0.0;

    shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;        

    if (currentDepth > 1.0)
        shadow = 0.0;

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

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 view_dir, float shadow) {
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

    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * u_object_color;

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
    vec3 half_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(half_dir, normal), 0.0), light.shininess);
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
    vec3 dp_dx = dFdx(frag_pos);
	vec3 dp_dy = dFdy(frag_pos);
    vec3 norm = normalize(cross(dp_dx, dp_dy));

    // first way of dealing with specular component and having uniform coloring inside the triangle; set the camera pos to some distant offset so that the view direction converges for all vertices
    /*
    vec3 camera_pos = vec3(inverse(u_view_trans)[3]);
    vec3 inf_offset = vec3(inverse(u_view_trans) * vec4(0., 0., pow(2, 20), 0.));
    camera_pos += inf_offset;
    */
    
    // second way, setting camera pos to distant z position; this should be the best solution as z is fixed and not an offset, therefore preventing out of bounds arithmetic
    mat4 inv_camera = inverse(u_view_trans);
    vec3 camera_pos = vec3(inv_camera * vec4(0.0, 0.0, pow(2, 10.0), 1.0));
    
    vec3 view_dir = normalize(vec3(inverse(u_view_trans)[3]) - frag_pos);

    float shadow = ShadowCalculation(frag_pos_light);

    vec3 lighting = CalcDirLight(dir_light, norm, camera_pos, shadow);
    // point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        lighting += CalcPointLight(point_lights[i], norm, frag_pos, camera_pos);

    vec3 shadow_result;
    if (bool(u_debug_shadows)) {
        if (shadow == 1.0) {
            shadow_result = vec3(1.0, 0.0, 0.0);
        } else {
            shadow_result = lighting;
        }
    } else {
        shadow_result = lighting;
    }

    out_color = vec4(shadow_result, 1.0);
}
#version 330 core
in vec3 frag_pos;
in vec3 normal;

uniform vec3 u_object_color;

uniform mat4 u_model_trans;
uniform mat4 u_view_trans;

out vec4 out_color;

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

    vec3 result = (ambient + diffuse + specular) * u_object_color;

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
    vec3 camera_pos = vec3(u_view_trans[3]);
    camera_pos.z = pow(2, 20);
    camera_pos = vec3(inverse(u_view_trans) * vec4(camera_pos, 1.0));
    
    vec3 view_dir = normalize(vec3(inverse(u_view_trans)[3]) - frag_pos);

    vec3 lighting = CalcDirLight(dir_light, norm, view_dir);
    // point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        lighting += CalcPointLight(point_lights[i], norm, frag_pos, view_dir);    
    out_color = vec4(lighting, 1.0);
}
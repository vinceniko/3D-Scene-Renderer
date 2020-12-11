#version 330 core
in vec3 frag_pos;
in vec3 normal;

uniform vec3 u_object_color;

uniform mat4 u_model_trans;
uniform mat4 u_view_trans;

out vec4 out_color;

struct DirLight {
    vec3 light_color;

    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform DirLight dir_light;

vec3 calc_dir_light(DirLight light, vec3 normal, vec3 view_dir) {
    vec3 ambient = light.ambient * light.light_color;

    float diff = max(dot(normal, light.direction), 0.0);
    vec3 diffuse = diff * light.light_color;

    // // phong
    // vec3 reflect_dir = reflect(-light.direction, normal);
    // float spec = pow(max(dot(view_dir, reflect_dir), 0.0), light.shininess);
    
    // blinn-phong
    vec3 half_dir = normalize(light.direction + view_dir);
    float spec = pow(max(dot(half_dir, normal), 0.0), light.shininess);
    vec3 specular = light.specular * spec * light.light_color;

    vec3 result = (ambient + diffuse + specular);

    return result;
}

/*
struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;

    float constant;
    float linear;
    float quadratic;
};  
#define NR_POINT_LIGHTS 4  
uniform PointLight pointLights[NR_POINT_LIGHTS];
*/

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

    vec3 lighting = calc_dir_light(dir_light, norm, view_dir);
    // point lights
    //for(int i = 0; i < NR_POINT_LIGHTS; i++)
    //    result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);    
    vec3 result = lighting * u_object_color;
    out_color = vec4(result, 1.0);
}
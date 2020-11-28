#version 330 core

in vec3 frag_pos;
in vec3 normal;

uniform vec3 u_object_color;

uniform mat4 u_model_trans;
uniform mat4 u_view_trans;

out vec4 out_color;

const vec3 light_pos = vec3(0.0, 20.0, 20.0);
const vec3 light_color = vec3(0.5);

const float ambient_strength = 0.5;

const float specular_strength = 0.5;

const float shininess = 128;

void main()
{
    vec3 ambient = ambient_strength * light_color;

    vec3 dp_dx = dFdx(frag_pos);
	vec3 dp_dy = dFdy(frag_pos);
    vec3 norm = normalize(cross(dp_dx, dp_dy));

    vec3 light_dir = normalize(light_pos - frag_pos);  
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;

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
    
    vec3 view_dir = normalize(camera_pos - frag_pos);

    // // phong
    // vec3 reflect_dir = reflect(-light_dir, norm);
    // float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);

    // blinn-phong
    vec3 halfDir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(halfDir, norm), 0.0), shininess);
    vec3 specular = specular_strength * spec * light_color;

    vec3 result = (ambient + diffuse + specular) * u_object_color;
    out_color = vec4(result, 1.0);
}
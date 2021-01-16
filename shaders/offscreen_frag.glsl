#version 330 core

in vec2 uv;

uniform sampler2D u_offscreen_tex;
uniform sampler2D u_depth_map;

out vec4 out_color;

// vignette
const float ellipse_factor = 0.8;
const float min_vignette_radius = 0.4;
const float outer_radius = 1.0;
const float gradient_fade_inner_radius = 0.4;
const float vignette_intensity = 0.92;

// fog
const vec3 fog_base = vec3(0.7);

void main()
{ 
    float n = 0.00001;
    float f = 100.0;

    float depth = texture(u_depth_map, uv).r;
    float z_ndc = 2.0 * depth - 1.0;
    float z_eye = 2.0 * n * f / (f + n - z_ndc * (f - n));
    
    out_color = texture(u_offscreen_tex, uv);
    
    // fog
    /*
    float fog_level = clamp(smoothstep(0.7, 1.0, 1.0 - z_eye), 0.1, 1.0);
    if (z_eye >= f) {
        fog_level = 0.1;
    } 
    */
    // b & w
    /*
    vec3 avg = vec3(0.2126 * out_color.r + 0.7152 * out_color.g + 0.0722 * out_color.b);
    out_color.rgb = avg;
    */
    // vignette
    /*
    vec2 uv_2 = (uv * 2) - 1;
    vec3 vignette = 1.0 - vec3(smoothstep(outer_radius - gradient_fade_inner_radius, outer_radius, length(uv_2) - min_vignette_radius));
    */
    // gamma correction
    out_color.rgb = pow(out_color.rgb, vec3(1 / 1.9));
    // fog mix
    // out_color.rgb = mix(out_color.rgb, fog_base, 1.0 - fog_level);
    // vignette mix
    // out_color.rgb = mix(out_color.rgb, out_color.rgb * vignette, vignette_intensity);

    // write to depth from tex
    gl_FragDepth = depth;
}
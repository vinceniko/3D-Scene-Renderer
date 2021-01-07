#version 330 core

in vec2 uv;

uniform sampler2D offscreen_tex;

out vec4 out_color;

// vignette
const float ellipse_factor = 0.8;
const float min_vignette_radius = 0.4;
const float outer_radius = 1.0;
const float gradient_fade_inner_radius = 0.4;
const float vignette_intensity = 0.92;

void main()
{ 
    out_color = texture(offscreen_tex, uv);
    // b & w
    vec3 avg = vec3(0.2126 * out_color.r + 0.7152 * out_color.g + 0.0722 *out_color.b);
    // vignette
    /*
    vec2 uv_2 = (uv * 2) - 1;
    vec3 vignette = 1.0 - vec3(smoothstep(outer_radius - gradient_fade_inner_radius, outer_radius, length(uv_2) - min_vignette_radius));
    out_color.rgb = avg;
    */
    // gamma correction
    out_color.rgb = pow(out_color.rgb, vec3(1 / 1.9));
    // color
    //out_color.rgb = mix(out_color.rgb, out_color.rgb * vignette, vignette_intensity);
}
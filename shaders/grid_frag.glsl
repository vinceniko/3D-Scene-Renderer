#version 330 core

in vec3 frag_pos;
out vec4 out_color;

void main() 
{
    // Pick a coordinate to visualize in a grid
    vec2 coord = frag_pos.xz;

    // Compute anti-aliased world-space grid lines
    coord *= 2.0; // frequency of lines
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / (abs(dFdx(coord)) + abs(dFdy(coord)));
    float line = min(grid.x, grid.y);

    // Just visualize the grid lines directly
    out_color = vec4(vec3(0.0), (1.0 - min(line, 1.0)));
}
#version 330 core

in vec2 tex_coord;

out vec4 FragColor;

uniform sampler2D texture_in;
uniform vec3 light_color_in;

void main()
{
    FragColor = vec4(light_color_in, 1.0) * texture(texture_in, tex_coord);
}

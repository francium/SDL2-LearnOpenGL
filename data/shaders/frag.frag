#version 330 core

in vec3 color;
in vec2 tex_coord;

out vec4 FragColor;

uniform sampler2D my_texture;

void main()
{
    FragColor = vec4(color, 1.0) * texture(my_texture, tex_coord);
}

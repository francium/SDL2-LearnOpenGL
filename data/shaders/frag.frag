#version 330 core

in vec2 tex_coord;

out vec4 FragColor;

uniform sampler2D texture_in;

void main()
{
    FragColor = texture(texture_in, tex_coord);
}

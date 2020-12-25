#version 330 core

in vec3 color;
in vec2 tex_coord;

out vec4 FragColor;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    FragColor = vec4(color, 1.0)
              * mix(
                  texture(texture1, tex_coord),
                  texture(texture2, tex_coord),
                  0.5
                );
}

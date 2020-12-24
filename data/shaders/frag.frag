#version 330 core

in vec3 color;
in vec2 tex_coord;

out vec4 FragColor;

uniform sampler2D texture1;
uniform sampler2D texture2;

uniform bool flip_x;

void main()
{
    vec2 a = vec2(
        flip_x
            ? -tex_coord.x
            : tex_coord.x,
        tex_coord.y
    );

    FragColor = vec4(color, 1.0)
              * mix(
                  texture(texture1, tex_coord),
                  texture(texture2, a),
                  0.5
                );
}

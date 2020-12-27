#version 330 core

in vec2 tex_coord;
in vec3 normal;
in vec3 frag_pos;

out vec4 FragColor;

uniform sampler2D texture_in;
uniform vec3 light_pos;
uniform vec3 light_color;

void main()
{
    // ambient
    float ambient_strength = 0.2;
    vec3 ambient = ambient_strength * light_color;

    // diffuse
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light_pos - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;

    // texture
    vec4 texture_color = texture(texture_in, tex_coord);

    FragColor = vec4((ambient + diffuse), 1.0) * texture_color;
}

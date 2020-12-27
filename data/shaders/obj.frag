#version 330 core

in vec2 tex_coord;
in vec3 normal;
in vec3 frag_pos;

out vec4 FragColor;

uniform sampler2D texture_in;
uniform vec3 light_pos;
uniform vec3 light_color;
uniform vec3 view_pos;

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

    // specular
    int shine_strength = 32;
    float specular_strength = 0.5;
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shine_strength);
    vec3 specular = specular_strength * spec * light_color;

    // texture
    vec4 texture_color = texture(texture_in, tex_coord);

    FragColor = vec4((ambient + diffuse + specular), 1.0) * texture_color;
}

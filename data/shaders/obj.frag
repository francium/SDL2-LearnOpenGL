#version 330 core

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shine;
};

struct Light
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec2 tex_coord;
in vec3 normal;
in vec3 frag_pos;

out vec4 FragColor;

uniform sampler2D texture_in;
uniform vec3 view_pos;
uniform Material material;
uniform Light light;

void main()
{
    // ambient
    vec3 ambient = light.ambient * material.ambient;

    // diffuse
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light.position - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    // specular
    int shine_strength = 32;
    float specular_strength = 0.5;
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shine);
    vec3 specular = light.specular * (spec * material.specular);

    // texture
    vec4 texture_color = texture(texture_in, tex_coord);

    FragColor = vec4((ambient + diffuse + specular), 1.0) * texture_color;
}

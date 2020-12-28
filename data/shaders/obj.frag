#version 330 core

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shine;
};

struct Light
{
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in vec2 tex_coord;
in vec3 normal;
in vec3 frag_pos;

out vec4 FragColor;

uniform vec3 view_pos;
uniform Material material;
uniform Light light;

void main()
{
    float distance = length(light.position - frag_pos);
    float inv_attenuation = (
        light.constant
        + light.linear * distance
        + light.quadratic * (distance * distance)
    );
    float attenuation = 1.0 / inv_attenuation;

    vec3 diffuse_map_value = vec3(texture(material.diffuse, tex_coord));
    vec3 specular_map_value = vec3(texture(material.specular, tex_coord));

    // diffuse
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(-light.direction);
    float diff = max(dot(norm, light_dir), 0.0);

    // specular
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shine);

    vec3 ambient = light.ambient * diffuse_map_value;
    vec3 diffuse = light.diffuse * (diff * diffuse_map_value);
    vec3 specular = light.specular * spec * specular_map_value;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}

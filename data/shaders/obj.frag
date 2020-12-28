#version 330 core

#define POINT_LINE_COUNT 2

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shine;
};

struct DirectionLight
{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;

    float cut_off;
    float outer_cut_off;

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
uniform DirectionLight direction_light;
uniform PointLight point_lights[POINT_LINE_COUNT];
uniform SpotLight spot_light;
uniform Material material;
uniform bool enable_sun;
uniform bool enable_point_lights;
uniform bool enable_flashlight;

float attenuation(vec3 light_position, float constant, float linear, float quadratic)
{
    float distance = length(light_position - frag_pos);
    float inv_attenuation = (
        constant
        + linear * distance
        + quadratic * (distance * distance)
    );
    return 1.0 / inv_attenuation;
}

vec3 calc_dir_light(DirectionLight light, vec3 normal, vec3 view_dir)
{
    vec3 light_dir = normalize(-light.direction);

    // diffuse
    float diff = max(dot(normal, light_dir), 0.0);

    // specular
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shine);

    vec3 diffuse_map_value = vec3(texture(material.diffuse, tex_coord));
    vec3 specular_map_value = vec3(texture(material.specular, tex_coord));

    vec3 ambient = light.ambient * diffuse_map_value;
    vec3 diffuse = light.diffuse * diff * diffuse_map_value;
    vec3 specular = light.specular * spec * specular_map_value;

    return ambient + diffuse + specular;
}

vec3 calc_point_light(PointLight light, vec3 normal, vec3 view_dir)
{
    vec3 light_dir = normalize(light.position - frag_pos);

    // diffuse
    float diff = max(dot(normal, light_dir), 0.0);

    // specular
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shine);

    float attenuation_value = attenuation(
        light.position, light.constant, light.linear, light.quadratic);

    vec3 diffuse_map_value = vec3(texture(material.diffuse, tex_coord));
    vec3 specular_map_value = vec3(texture(material.specular, tex_coord));

    vec3 ambient = attenuation_value * light.ambient * diffuse_map_value;
    vec3 diffuse = attenuation_value * light.diffuse * diff * diffuse_map_value;
    vec3 specular = attenuation_value * light.specular * spec * specular_map_value;

    return ambient + diffuse + specular;
}

vec3 calc_spot_light(SpotLight light, vec3 normal, vec3 view_dir)
{
    vec3 light_dir = normalize(light.position - frag_pos);

    // diffuse
    float diff = max(dot(normal, light_dir), 0.0);

    // specular
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shine);

    float attenuation_value = attenuation(
        light.position, light.constant, light.linear, light.quadratic);

    float theta = dot(light_dir, normalize(-light.direction));
    float epsilon = light.cut_off - light.outer_cut_off;
    float intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0, 1.0);

    vec3 diffuse_map_value = vec3(texture(material.diffuse, tex_coord));
    vec3 specular_map_value = vec3(texture(material.specular, tex_coord));

    vec3 ambient = intensity * attenuation_value * light.ambient * diffuse_map_value;
    vec3 diffuse = intensity * attenuation_value * light.diffuse * diff * diffuse_map_value;
    vec3 specular = intensity * attenuation_value * light.specular * spec * specular_map_value;

    return ambient + diffuse + specular;
}

void main()
{
    vec3 norm = normalize(normal);
    vec3 view_dir = normalize(view_pos - frag_pos);

    vec3 result = vec3(0.0);

    if (enable_sun)
        result += calc_dir_light(direction_light, norm, view_dir);

    if (enable_point_lights)
        for (int i = 0; i < POINT_LINE_COUNT; i++)
            result += calc_point_light(point_lights[i], norm, view_dir);

    if (enable_flashlight)
        result += calc_spot_light(spot_light, norm, view_dir);

    FragColor = vec4(result, 1.0);
}

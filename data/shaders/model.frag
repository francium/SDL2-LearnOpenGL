#version 330 core

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shine;
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

struct DirectionLight
{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec2 frag_tex_coord;
in vec3 frag_pos;
in vec3 frag_normal;

out vec4 frag_color;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

uniform vec3 view_pos;
uniform vec3 light_dir;

uniform Material material;

uniform DirectionLight direction_light;
uniform bool enable_sun;

uniform SpotLight spot_light;
uniform bool enable_flashlight;

float attenuation(vec3 light_position, float constant, float linear, float quadratic);
vec3 calc_dir_light(DirectionLight light, vec3 normal, vec3 view_dir);
void calc_point_light(PointLight light, vec3 normal, vec3 view_dir);
vec3 calc_spot_light(SpotLight light, vec3 normal, vec3 view_dir);

void main()
{
    // vec3 norm = normalize(frag_normal);
    vec3 norm = texture(texture_normal1, frag_tex_coord).xyz;
    vec3 view_dir = normalize(view_pos - frag_pos);

    vec3 result = vec3(0.0);

    if (enable_sun)
        result += calc_dir_light(direction_light, norm, view_dir);

    if (enable_flashlight)
    {
        result += calc_spot_light(spot_light, norm, view_dir);
    }
    // if (result.x < 0.25)
    //     result = vec3(1.0, 0.0, 0.0);
    // if (result.y < 0.25)
    //     result = vec3(0.0, 1.0, 0.0);
    // if (result.y < 0.25)
    //     result = vec3(0.0, 0.0, 1.0);

    frag_color = vec4(result, 1.0);
}

vec3 calc_dir_light(DirectionLight light, vec3 normal, vec3 view_dir)
{
    vec3 light_dir = normalize(-light.direction);

    // diffuse
    float diff = max(dot(normal, light_dir), 0.0);

    // // specular
    vec3 reflect_dir = reflect(-light_dir, normal);
    float material_shine = 4.0;
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material_shine);

    vec3 diffuse_color = texture(texture_diffuse1, frag_tex_coord).xyz;
    // vec3 specular_intensity = texture(texture_specular1, frag_tex_coord).xyz;
    vec3 specular_intensity = vec3(0.5, 0.5, 0.5);

    vec3 ambient = light.ambient * diffuse_color;
    vec3 diffuse = light.diffuse * diff * diffuse_color;
    vec3 specular = light.specular * spec * specular_intensity;

    return ambient + diffuse + specular;
}

void calc_point_light(PointLight light, vec3 normal, vec3 view_dir)
{
    // vec3 light_dir = normalize(light.position - frag_pos);
    //
    // // diffuse
    // float diff = max(dot(normal, light_dir), 0.0);
    //
    // // specular
    // vec3 reflect_dir = reflect(-light_dir, normal);
    // float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shine);
    //
    // float attenuation_value = attenuation(
    //     light.position, light.constant, light.linear, light.quadratic);
    //
    // vec3 diffuse_map_value = vec3(texture(material.diffuse, tex_coord));
    // vec3 specular_map_value = vec3(texture(material.specular, tex_coord));
    //
    // vec3 ambient = attenuation_value * light.ambient * diffuse_map_value;
    // vec3 diffuse = attenuation_value * light.diffuse * diff * diffuse_map_value;
    // vec3 specular = attenuation_value * light.specular * spec * specular_map_value;
    //
    // return ambient + diffuse + specular;
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

    vec3 diffuse_color = texture(texture_diffuse1, frag_tex_coord).xyz;
    vec3 specular_intensity = texture(texture_specular1, frag_tex_coord).xyz;

    vec3 ambient = intensity * attenuation_value * light.ambient * diffuse_color;
    vec3 diffuse = intensity * attenuation_value * light.diffuse * diff * diffuse_color;
    vec3 specular = intensity * attenuation_value * light.specular * spec * specular_intensity;
    // return intensity
    // specular = vec3(0.0);

    return ambient + diffuse + specular;
}

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


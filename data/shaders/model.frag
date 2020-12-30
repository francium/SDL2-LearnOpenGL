#version 330 core
out vec4 frag_color;

in vec2 tex_coord;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform sampler2D texture_normal;
uniform sampler2D texture_height;

void main()
{
    vec4 diffuse_color = texture(texture_diffuse, tex_coord);
    vec4 specular_intensity = texture(texture_specular, tex_coord);
    frag_color = diffuse_color * specular_intensity;
}

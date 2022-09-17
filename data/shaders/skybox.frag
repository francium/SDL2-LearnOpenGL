#version 330 core

out vec4 frag_color;

in vec3 tex_coord;

uniform samplerCube skybox;

void main()
{
    // frag_color = texture(skybox, tex_coord);
    frag_color = vec4(tex_coord.xy, 1.0, 1.0);
    // frag_color = vec4(1.0, 0.0, 0.0, 1.0);
}

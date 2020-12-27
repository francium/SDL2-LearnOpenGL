#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex_coord_in;
layout (location = 2) in vec3 normal_in;

out vec3 color;
out vec2 tex_coord;
out vec3 normal;
out vec3 frag_pos;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main()
{
    tex_coord = tex_coord_in;
    normal = normal_in;
    frag_pos = vec3(model_matrix * vec4(pos, 1.0));
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(pos, 1.0);
}

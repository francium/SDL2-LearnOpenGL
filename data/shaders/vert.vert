#version 330 core

layout (location = 0) in vec3 vert_pos;
layout (location = 1) in vec3 vert_normal;
layout (location = 2) in vec2 vert_tex_coord;
layout (location = 3) in vec2 vert_tangent;
layout (location = 4) in vec2 vert_bitangent;

out vec2 frag_tex_coord;
out vec3 frag_pos;
out vec3 frag_normal;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main()
{
    frag_tex_coord = vert_tex_coord;
    frag_pos = vec3(model_matrix * vec4(vert_pos, 1.0));
    frag_normal = vert_normal;

    gl_Position = projection_matrix * view_matrix * vec4(frag_pos, 1.0);
}

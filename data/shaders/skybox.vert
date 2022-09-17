#version 330 core

layout (location = 0) in vec3 pos;

out vec3 tex_coord;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    tex_coord = pos;
    // vec4 final_pos = projection * view * vec4(pos, 1.0);
    // gl_Position = final_pos.xyww;
    gl_Position = vec4(pos.xyz, 1.0);
}

#pragma once


#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <glad/glad.h>

#include "shader_utils.hpp"


struct Shader
{
    GLuint id;
};


struct ShadersCompilationResult
{
    bool success;
    GLuint vertex_shader;
    GLuint fragment_shader;
};


internal ShadersCompilationResult
compile_shaders_from_files(
    const char *vertex_shader_path,
    const char *fragment_shader_path
)
{
    char *vertex_shader_buffer = read_file(vertex_shader_path);
    if (vertex_shader_buffer == nullptr)
    {
        fprintf(stderr, "Failed to open shader source: %s\n", vertex_shader_path);
        return {false};
    }

    char *fragment_shader_buffer = read_file(fragment_shader_path);
    if (fragment_shader_buffer == nullptr)
    {
        fprintf(stderr, "Failed to open shader source: %s\n", fragment_shader_path);
        return {false};
    }

    int success;

    ShaderCompilationResult vertex_shader_result = compile_shader(
        GL_VERTEX_SHADER, vertex_shader_buffer);
    if (!vertex_shader_result.success)
    {
        fprintf(stderr, "Failed to compile shader source: %s\n", vertex_shader_path);
        return {false};
    }
    fprintf(stderr, "Compiled shader: %s\n", vertex_shader_path);
    GLuint vertex_shader = vertex_shader_result.shader;

    ShaderCompilationResult fragment_shader_result = compile_shader(
        GL_FRAGMENT_SHADER, fragment_shader_buffer);
    if (!fragment_shader_result.success)
    {
        fprintf(stderr, "Failed to compile shader source: %s\n", fragment_shader_path);
        return {false};
    }
    fprintf(stderr, "Compiled shader: %s\n", fragment_shader_path);
    GLuint fragment_shader = fragment_shader_result.shader;

    free(vertex_shader_buffer);
    free(fragment_shader_buffer);

    return {true, vertex_shader, fragment_shader};
}


internal bool
Shader_init(
    Shader *shader,
    const char *vertex_shader_path,
    const char *fragment_shader_path
)
{
    ShadersCompilationResult shaders = compile_shaders_from_files(
        vertex_shader_path, fragment_shader_path);
    if (!shaders.success)
    {
        return false;
    }

    shader->id = glCreateProgram();
    glAttachShader(shader->id, shaders.vertex_shader);
    glAttachShader(shader->id, shaders.fragment_shader);
    glLinkProgram(shader->id);

    int success;
    glGetProgramiv(shader->id, GL_LINK_STATUS, &success);
    if (!success) {
        log_shader_link_error(shader->id);
        glDeleteProgram(shader->id);
        cleanup_shaders(shaders.vertex_shader, shaders.fragment_shader);
        return false;
    }

    cleanup_shaders(shaders.vertex_shader, shaders.fragment_shader);

    return true;
}


/** Activate shader */
void Shader_use(Shader *shader)
{
    glUseProgram(shader->id);
}


void Shader_destroy(Shader *shader)
{
    glDeleteProgram(shader->id);
}


void Shader_seti(Shader *shader, const char *name, int value)
{
    glUniform1i(glGetUniformLocation(shader->id, name), value);
}


void Shader_setf(Shader *shader, const char *name, f32 value)
{
    glUniform1f(glGetUniformLocation(shader->id, name), value);
}


void Shader_setv3(Shader *shader, const char *name, f32 x, f32 y, f32 z)
{
    glUniform3f(glGetUniformLocation(shader->id, name), x, y, z);
}


void Shader_set_matrix4fv(Shader *shader, const char *name, const GLfloat *value)
{
    GLuint uniform_location = glGetUniformLocation(shader->id, name);
    glUniformMatrix4fv(
        uniform_location,
        1,
        GL_FALSE,
        value
    );
}

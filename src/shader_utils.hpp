#pragma once


#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <glad/glad.h>


struct ShaderCompilationResult
{
    GLuint shader;
    bool success;
};


internal char *
read_file(const char *file_path)
{
    FILE *fh = fopen(file_path, "rb");
    if (fh == nullptr)
    {
        fprintf(stderr, "Failed to open file: %s\n", file_path);
        return nullptr;
    }

    int fd = fileno(fh);
    struct stat file_stat;
    fstat(fd, &file_stat);
    off_t file_size = file_stat.st_size + 1; // + 1 for appending '\0'

    char *buffer = (char *)malloc(sizeof(char) * file_size);
    buffer[file_size - 1] = '\0';
    fread(buffer, sizeof(char) * file_size, 1, fh);
    fclose(fh);

    return buffer;
}


internal void
log_shader_link_error(GLuint program)
{
    char info_log[512];
    glGetProgramInfoLog(program, 512, nullptr, info_log);
    fprintf(stderr, "Failed to link shaders:\n%s\n", info_log);
}


internal void
log_shader_compilation_error(GLuint shader)
{
    char info_log[512];
    glGetShaderInfoLog(shader, 512, nullptr, info_log);
    fprintf(stderr, "Failed to compile shader:\n%s\n", info_log);
}


internal void
cleanup_shaders(GLuint frag, GLuint vert)
{
    glDeleteShader(vert);
    glDeleteShader(frag);
}


internal ShaderCompilationResult
compile_shader(int shader_type, const char *source)
{
    ShaderCompilationResult result = {};

    result.shader = glCreateShader(shader_type);
    glShaderSource(result.shader, 1, &source, nullptr);
    glCompileShader(result.shader);

    GLint success;
    glGetShaderiv(result.shader, GL_COMPILE_STATUS, (GLint *)&result.success);
    if (!result.success) log_shader_compilation_error(result.shader);

    return result;
}

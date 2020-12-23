#include <stdio.h>

#include <SDL2/SDL.h>
#include <glad/glad.h>

#include "platform.hpp"


struct App
{
    SDL_Window    *window;
    SDL_GLContext  context;
    GLuint         ebo,
                   vao,
                   vbo;
    GLuint         vertex_shader;
    GLuint         fragment_shader_1;
    GLuint         fragment_shader_2;
    GLuint         shader_program_1;
    GLuint         shader_program_2;
};


const float vertices[] = {
    0.0f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.5f, 0.0f, 0.0f,
    0.0f, 0.5f, 0.0f,
    -0.5f, 0.5f, 0.0f,
    -0.5f, 0.0f, 0.0f,
};

const u32 indices[] = {
    0, 1, 2, // First triangle
    3, 4, 5, // Second triangle
};

const char *vertex_shader_source = R"shader(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    void main()
    {
        gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    }
)shader";

const char *fragment_shader_1_source = R"shader(
    #version 330 core
    out vec4 FragColor;

    void main()
    {
        FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    }
)shader";

const char *fragment_shader_2_source = R"shader(
    #version 330 core
    out vec4 FragColor;

    void main()
    {
        FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
)shader";


internal bool
init_rendering_context(App *app)
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE
    );

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    app->context = SDL_GL_CreateContext(app->window);
    if (app->context == nullptr)
    {
        printf("Failed to create GL context\n");
        return false;
    }

    SDL_GL_SetSwapInterval(1); // Use VSYNC

    return true;
}


internal bool
init_sdl(App *app)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "Failed to initialize SDL video\n");
        return false;
    }

    app->window = SDL_CreateWindow(
        "SDL App",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640, 480,
        SDL_WINDOW_OPENGL);
    if (app->window == nullptr)
    {
        fprintf(stderr, "Failed to create main window\n");
        return false;
    }

    return init_rendering_context(app);
}


internal bool
init_gl(App *app)
{
    int gladInitRes = gladLoadGL();
    if (!gladInitRes)
    {
        fprintf(stderr, "Unable to initialize glad\n");
        return false;
    }

    return true;
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
cleanup_shaders(App *app)
{
    glDeleteShader(app->vertex_shader);
    glDeleteShader(app->fragment_shader_1);
}


internal void
cleanup_gl(App *app)
{
    glDeleteVertexArrays(1, &app->vao);
    glDeleteBuffers(1, &app->vbo);
    glDeleteProgram(app->shader_program_1);
    glDeleteProgram(app->shader_program_2);
}


internal void
cleanup_sdl(App *app)
{
    SDL_GL_DeleteContext(app->context);
    SDL_DestroyWindow(app->window);
    SDL_Quit();
}


internal void
cleanup(App *app)
{
    cleanup_gl(app);
    cleanup_sdl(app);
}


internal bool
compile_shader(GLuint shader, const char *source)
{
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) log_shader_compilation_error(shader);

    return success;
}


internal bool
compile_vertex_shader(App *app)
{
    app->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    return compile_shader(app->vertex_shader, vertex_shader_source);
}


internal bool
compile_fragment_shader(App *app)
{
    app->fragment_shader_1 = glCreateShader(GL_FRAGMENT_SHADER);
    const bool result1 = compile_shader(app->fragment_shader_1, fragment_shader_1_source);

    app->fragment_shader_2 = glCreateShader(GL_FRAGMENT_SHADER);
    const bool result2 = compile_shader(app->fragment_shader_2, fragment_shader_2_source);

    return result1 && result2;
}


internal bool
compile_shaders(App *app)
{
    if (!compile_vertex_shader(app)) return false;
    if (!compile_fragment_shader(app)) return false;

    return true;
}


internal bool
link_shaders(App *app)
{
    if (!compile_shaders(app)) return false;

    {
        app->shader_program_1 = glCreateProgram();
        glAttachShader(app->shader_program_1, app->vertex_shader);
        glAttachShader(app->shader_program_1, app->fragment_shader_1);
        glLinkProgram(app->shader_program_1);

        GLint success;
        glGetProgramiv(app->shader_program_1, GL_LINK_STATUS, &success);
        if (!success) {
            log_shader_link_error(app->shader_program_1);
            glDeleteProgram(app->shader_program_1);
            cleanup_shaders(app);
            return false;
        }
    }

    {
        app->shader_program_2 = glCreateProgram();
        glAttachShader(app->shader_program_2, app->vertex_shader);
        glAttachShader(app->shader_program_2, app->fragment_shader_2);
        glLinkProgram(app->shader_program_2);

        GLint success;
        glGetProgramiv(app->shader_program_2, GL_LINK_STATUS, &success);
        if (!success) {
            log_shader_link_error(app->shader_program_2);
            glDeleteProgram(app->shader_program_2);
            cleanup_shaders(app);
            return false;
        }
    }

    cleanup_shaders(app);

    return true;
}


internal void
init_rendering_data(App *app)
{
    glGenVertexArrays(1, &app->vao);
    glGenBuffers(1, &app->vbo);
    glGenBuffers(1, &app->ebo);

    glBindVertexArray(app->vao);
    {
        glBindBuffer(GL_ARRAY_BUFFER, app->vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), nullptr);
        glEnableVertexAttribArray(0);
    }
}


internal bool
init(App *app)
{
    if (!init_sdl(app)) return false;
    if (!init_gl(app)) return false;
    if (!link_shaders(app)) return false;

    init_rendering_data(app);

    return true;
}


internal void
update(App *app)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(app->shader_program_1);
    glBindVertexArray(app->vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    {
        glUseProgram(app->shader_program_2);
        glBindVertexArray(app->vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    SDL_GL_SwapWindow(app->window);
}


/** Return of a `true` indicates program should exit */
internal bool
process_events()
{
    bool quit = false;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type)
        {
            case SDL_KEYDOWN:
            {
                quit = true;
            } break;

            case SDL_WINDOWEVENT:
            {
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    quit = true;
                }
            } break;
        }
    }

    return quit;
}


int main(int argc, char *argv[])
{
    App app = {};

    printf("Initializing...\n");
    if (!init(&app))
    {
        cleanup(&app);
        return 1;
    }

    printf("Starting...\n");

    bool should_run = true;
    while (should_run)
    {
        const bool quit = process_events();
        if (quit) break;

        update(&app);
    }

    printf("Exiting...\n");
    cleanup(&app);

    return 0;
}

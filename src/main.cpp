#include <stdio.h>

#include <SDL2/SDL.h>
#include <glad/glad.h>

#include "platform.hpp"


struct App
{
    SDL_Window    *window;
    SDL_GLContext  context;
    GLuint         vao,
                   vbo,
                   ebo,
                   tex;
    GLuint         vert_shader;
    GLuint         frag_shader;
    GLuint         shader_prog;
};


void log(const char *msg)
{
    printf(msg);
}


void log_err(const char *msg)
{
    fprintf(stderr, msg);
}


bool init_rendering_context(App *app)
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE
    );

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    app->context = SDL_GL_CreateContext(app->window);
    if (app->context == NULL)
    {
        log_err("Failed to create GL context\n");
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        return false;
    }

    SDL_GL_SetSwapInterval(1); // Use VSYNC

    return true;
}


bool init_sdl(App *app)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        log_err("Failed to initialize SDL video\n");
        return false;
    }

    app->window = SDL_CreateWindow(
        "SDL App",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640, 480,
        SDL_WINDOW_OPENGL);
    if (app->window == NULL)
    {
        log_err("Failed to create main window\n");
        SDL_Quit();
        return false;
    }

    return init_rendering_context(app);
}


bool init_gl(App *app)
{
    int gladInitRes = gladLoadGL();
    if (!gladInitRes)
    {
        log_err("Unable to initialize glad\n");
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        return false;
    }

    return true;
}


bool init(App *app)
{
    if (!init_sdl(app))
    {
        return false;
    }

    if (!init_gl(app))
    {
        return false;
    }

    return true;
}


void cleanup(App *app)
{
    glUseProgram(0);
    glDisableVertexAttribArray(0);
    glDetachShader(app->shader_prog, app->vert_shader);
    glDetachShader(app->shader_prog, app->frag_shader);
    glDeleteProgram(app->shader_prog);
    glDeleteShader(app->vert_shader);
    glDeleteShader(app->frag_shader);
    glDeleteTextures(1, &app->tex);
    glDeleteBuffers(1, &app->ebo);
    glDeleteBuffers(1, &app->vbo);
    glDeleteVertexArrays(1, &app->vao);
    SDL_GL_DeleteContext(app->context);
    SDL_DestroyWindow(app->window);
    SDL_Quit();
}


void update(App *app)
{
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(app->window);
}


int main(int argc, char *argv[])
{
    App app = {};

    log("Initializing...\n");
    if (!init(&app))
    {
        return 1;
    }

    log("Starting...\n");

    bool should_run = true;
    while (should_run)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_WINDOWEVENT
                && event.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                should_run = 0;
                break;
            }
        }

        update(&app);
    }

    log("Exiting...\n");
    cleanup(&app);

    return 0;
}

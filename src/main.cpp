#include <stdio.h>

#include <SDL2/SDL.h>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "platform.hpp"
#include "shader.hpp"


struct Texture
{
    GLuint id;
};


internal Result<Texture>
Texture_load(const char *path)
{
    int width, height, nr_channels;
    u8 *data = stbi_load(path, &width, &height, &nr_channels, 0);
    if (data == nullptr)
    {
        return {.ok = false};
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        width,
        height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        data
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return {
        .ok = true,
        .value = {texture},
    };
}


internal void
Texture_use(Texture *texture)
{
    glBindTexture(GL_TEXTURE_2D, texture->id);
}


struct App
{
    SDL_Window    *window;
    SDL_GLContext  context;
    GLuint         ebo,
                   vao,
                   vbo;
    const char     *frag_shader_path,
                   *vert_shader_path,
                   *texture_path;
    Shader         shader;
    Texture        texture;
};


const int stride = 8;

const float vertices[] = {
    // top right
    0.5f,  0.5f, 0.0f,
    1.0f, 0.5f, 0.0f,
    1.0f, 1.0f,

    // bottom right
    0.5f, -0.5f, 0.0f,
    0.5f, 1.0f, 0.0f,
    1.0f, 0.0f,

    // bottom left
    -0.5f, -0.5f, 0.0f,
    0.0f, 0.5f, 1.0f,
    0.0f, 0.0f,

    // top left
    -0.5f,  0.5f, 0.0f,
    1.0f, 1.0f, 1.0f,
    0.0f, 1.0f,
};

const u32 indices[] = {
    0, 1, 3, // First triangle
    1, 2, 3, // Second triangle
};

const f32 tex_coords[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.5f, 1.0f,
};


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
        600, 600,
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
cleanup_gl(App *app)
{
    glDeleteVertexArrays(1, &app->vao);
    glDeleteBuffers(1, &app->vbo);
    Shader_destroy(&app->shader);
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

        // Vertex position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(f32), nullptr);
        glEnableVertexAttribArray(0);

        // Vertex color
        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            stride * sizeof(f32),
            (void *)(3 * sizeof(f32))
        );
        glEnableVertexAttribArray(1);

        // Tex coord
        glVertexAttribPointer(
            2,
            2,
            GL_FLOAT,
            GL_FALSE,
            stride * sizeof(f32),
            (void *)(6 * sizeof(f32))
        );
        glEnableVertexAttribArray(2);
    }
}


internal bool
init_shaders(App *app)
{
    bool shader_init_ok = Shader_init(
        &app->shader,
        app->vert_shader_path,
        app->frag_shader_path
    );
    if (!shader_init_ok) return false;

    return true;
}


internal bool
load_textures(App *app)
{
    Result<Texture> r_texture = Texture_load(app->texture_path);
    if (!r_texture.ok)
    {
        fprintf(stderr, "Failed to load textures\n");
        return false;
    }

    app->texture = r_texture.value;

    fprintf(stderr, "Loaded textures\n");

    return true;
}


internal bool
init(App *app)
{
    if (!init_sdl(app)) return false;
    if (!init_gl(app)) return false;
    if (!init_shaders(app)) return false;
    if (!load_textures(app)) return false;

    init_rendering_data(app);

    return true;
}


internal void
update(App *app)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    Shader_use(&app->shader);
    Texture_use(&app->texture);
    glBindVertexArray(app->vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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
                if (event.key.keysym.sym == SDLK_q)
                {
                    quit = true;
                }
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
    App app = {
        .frag_shader_path = "data/shaders/frag.frag",
        .vert_shader_path = "data/shaders/vert.vert",
        .texture_path = "data/textures/stone.jpg",
    };
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

#include <stdio.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "platform.hpp"
#include "result.hpp"
#include "shader.hpp"
#include "texture.hpp"


struct App
{
    u32            window_height,
                   window_width;
    SDL_Window     *window;
    SDL_GLContext  context;
    GLuint         ebo,
                   vao,
                   vbo;
    const char     *frag_shader_path,
                   *vert_shader_path,
                   *texture1_path,
                   *texture2_path;
    Shader         shader;
    Texture        texture1,
                   texture2;
};


const int stride = 5;

const float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

const u32 indices[] = {
    0, 1, 3, // First triangle
    1, 2, 3, // Second triangle
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
        app->window_width,
        app->window_height,
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

    glEnable(GL_DEPTH_TEST);

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

    glBindVertexArray(app->vao);
    {
        glBindBuffer(GL_ARRAY_BUFFER, app->vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Vertex position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(f32), nullptr);
        glEnableVertexAttribArray(0);

        // Tex coord
        glVertexAttribPointer(
            1,
            2,
            GL_FLOAT,
            GL_FALSE,
            stride * sizeof(f32),
            (void *)(3 * sizeof(f32))
        );
        glEnableVertexAttribArray(1);
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
load_texture(const char *path, Texture *dest)
{
    Result<Texture> r_texture_1 = Texture_load(path);
    if (!r_texture_1.ok)
    {
        fprintf(stderr, "Failed to load texture: %s\n", path);
        return false;
    }

    *dest = r_texture_1.value;
    fprintf(stderr, "Loaded texture: %s\n", path);

    return true;
}


internal bool
load_textures(App *app)
{
    if (!load_texture(app->texture1_path, &app->texture1)) return false;
    if (!load_texture(app->texture2_path, &app->texture2)) return false;

    Shader_use(&app->shader);

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
    struct timespec clock;
    clock_gettime(CLOCK_MONOTONIC, &clock);

    f32 t = (f32)clock.tv_sec + (f32)clock.tv_nsec / 1e9;
    f32 rotation_speed = 0.5;

    glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);

    f32 radius = 10.0f;
    f32 cam_x = sin(rotation_speed * t) * radius;
    f32 cam_z = cos(rotation_speed * t) * radius;
    f32 cam_y = 5.0f;

    glm::vec3 camera_pos = glm::vec3(cam_x, cam_y, cam_z);
    glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 camera_direction = glm::normalize(camera_pos - camera_target);
    glm::vec3 camera_right = glm::normalize(glm::cross(world_up, camera_direction));
    glm::vec3 camera_up = glm::cross(camera_direction, camera_right);

    glm::mat4 view_matrix = glm::lookAt(camera_pos, camera_target, world_up);
    GLuint view_matrix_uniform = glGetUniformLocation(app->shader.id, "view_matrix");
    glUniformMatrix4fv(view_matrix_uniform, 1, GL_FALSE, glm::value_ptr(view_matrix));

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Shader_use(&app->shader);

    glBindVertexArray(app->vao);

    glm::mat4 projection_matrix = glm::perspective(
        glm::radians(45.0f),
        (f32)app->window_width / (f32)app->window_height,
        0.1f,
        100.0f
    );
    GLuint projection_matrix_uniform = glGetUniformLocation(app->shader.id, "projection_matrix");
    glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, glm::value_ptr(projection_matrix));

    u32 num_objects = 3;
    glm::vec3 obj_positions[] = {
        glm::vec3(-1.25f, 0.5f, 0.0f),
        glm::vec3(0.0f, 0.5f, -1.5f),
        glm::vec3(1.5f, 0.5f, 0.0f),
    };

    // floor
    {
        Texture_use(&app->texture1, GL_TEXTURE0);
        glm::mat4 model_matrix = glm::mat4(1.0f);
        model_matrix = glm::translate(model_matrix, glm::vec3(0.0f, 0.0f, 0.0f));
        model_matrix = glm::scale(model_matrix, glm::vec3(50.0f, 0.0f, 50.0f));
        GLuint model_matrix_uniform = glGetUniformLocation(app->shader.id, "model_matrix");
        glUniformMatrix4fv(model_matrix_uniform, 1, GL_FALSE, glm::value_ptr(model_matrix));

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    Texture_use(&app->texture2, GL_TEXTURE0);
    for (u32 i = 0; i < num_objects; i++)
    {
        glm::mat4 model_matrix = glm::mat4(1.0f);
        model_matrix = glm::translate(model_matrix, obj_positions[i]);
        model_matrix = glm::rotate(model_matrix, glm::radians(90.0f * i), glm::vec3(0.0f, 1.0f, 0.0f));
        GLuint model_matrix_uniform = glGetUniformLocation(app->shader.id, "model_matrix");
        glUniformMatrix4fv(model_matrix_uniform, 1, GL_FALSE, glm::value_ptr(model_matrix));

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    SDL_GL_SwapWindow(app->window);
}


/** Return of a `true` indicates program should exit */
internal bool
process_events(App *app)
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


int
main(int argc, char *argv[])
{
    App app = {
        .window_height = 600,
        .window_width = 600,
        .frag_shader_path = "data/shaders/frag.frag",
        .vert_shader_path = "data/shaders/vert.vert",
        .texture1_path = "data/textures/stone.png",
        .texture2_path = "data/textures/coin.png",
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
        const bool quit = process_events(&app);
        if (quit) break;

        update(&app);
    }

    printf("Exiting...\n");
    cleanup(&app);

    return 0;
}

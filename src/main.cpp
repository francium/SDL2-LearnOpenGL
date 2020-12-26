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


const f32 default_camera_height = 1.0f;
const f32 default_fov = 45.0f;

bool first_motion_input = false;
bool first_mouse_wheel_input = false;


struct Camera
{
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    f32 yaw;
    f32 pitch;
    f32 fov;
};


internal void
Camera_init(Camera *c)
{
    c->position = glm::vec3(0.0f, default_camera_height, 5.0f);
    c->front = glm::vec3(0.0f, 0.0f, -1.0f);
    c->up = glm::vec3(0.0f, 1.0f, 0.0f);
    c->yaw = 0.0f;
    c->pitch = 0.0f;
    c->fov = default_fov;
}


struct Clock
{
    f32 dt;
    struct timespec last_tick;
};


internal void
Clock_init(Clock *clock)
{
    clock_gettime(CLOCK_MONOTONIC, &clock->last_tick);
}


internal void
Clock_tick(Clock *clock)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    clock->dt = (f32)(now.tv_sec - clock->last_tick.tv_sec)
              + (f32)(now.tv_nsec - clock->last_tick.tv_nsec) / 1e6;
    // FIXME: For some reason dt becomes negative (now < last_tick or this math
    // above to calculate dt is wrong)
    clock->dt = fmax(0, clock->dt);
    clock->last_tick = now;
}


struct Inputs
{
    bool forward;
    bool backward;
    bool left;
    bool right;
};


struct App
{
    u32            window_width,
                   window_height;
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
    Clock          clock;
    Inputs         inputs;
    Camera         camera;
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


internal f32
clamp(f32 min, f32 value, f32 max)
{
    return fmax(min, fmin(max, value));
}


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
        SDL_WINDOW_INPUT_GRABBED | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
    if (app->window == nullptr)
    {
        fprintf(stderr, "Failed to create main window\n");
        return false;
    }

    SDL_ShowCursor(SDL_DISABLE);
    SDL_GL_SetSwapInterval(1); // Use VSYNC

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
    Clock_tick(&app->clock);

    glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::mat4 view_matrix = glm::lookAt(app->camera.position, app->camera.position + app->camera.front, app->camera.up);
    GLuint view_matrix_uniform = glGetUniformLocation(app->shader.id, "view_matrix");
    glUniformMatrix4fv(view_matrix_uniform, 1, GL_FALSE, glm::value_ptr(view_matrix));

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Shader_use(&app->shader);

    glBindVertexArray(app->vao);

    glm::mat4 projection_matrix = glm::perspective(
        glm::radians(app->camera.fov),
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
    bool mouse_checked = false;
    f32 mouse_xd;
    f32 mouse_yd;
    f32 mouse_wheel_y;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type)
        {
            case SDL_MOUSEMOTION:
            {
                if (first_motion_input)
                {
                    first_motion_input = true;
                    break;
                }

                // Make sure we don't for another motion event as result of centering mouse
                if (mouse_checked) break;

                mouse_xd = (f32)event.motion.xrel;
                mouse_yd = -(f32)event.motion.yrel;

                // Center mouse and set flag to avoid checking the generated
                // motion event
                mouse_checked = true;
                SDL_WarpMouseInWindow(
                    app->window,
                    app->window_width / 2,
                    app->window_height / 2
                );
            } break;

            case SDL_MOUSEWHEEL:
            {
                if (first_mouse_wheel_input)
                {
                    first_mouse_wheel_input = true;
                    break;
                }

                mouse_wheel_y = -(f32)event.wheel.y;
            } break;

            case SDL_KEYDOWN:
            {
                if (event.key.keysym.sym == SDLK_q)
                {
                    quit = true;
                }
                if (event.key.keysym.sym == SDLK_w)
                {
                    app->inputs.forward = true;
                }
                if (event.key.keysym.sym == SDLK_s)
                {
                    app->inputs.backward = true;
                }
                if (event.key.keysym.sym == SDLK_a)
                {
                    app->inputs.left = true;
                }
                if (event.key.keysym.sym == SDLK_d)
                {
                    app->inputs.right = true;
                }
            } break;

            case SDL_KEYUP:
            {
                if (event.key.keysym.sym == SDLK_w)
                {
                    app->inputs.forward = false;
                }
                if (event.key.keysym.sym == SDLK_s)
                {
                    app->inputs.backward = false;
                }
                if (event.key.keysym.sym == SDLK_a)
                {
                    app->inputs.left = false;
                }
                if (event.key.keysym.sym == SDLK_d)
                {
                    app->inputs.right = false;
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

    f32 camera_speed = app->clock.dt * 0.005f;

    if (app->inputs.forward)
    {
        app->camera.position += camera_speed * app->camera.front;
    }
    if (app->inputs.backward)
    {
        app->camera.position -= camera_speed * app->camera.front;
    }
    if (app->inputs.left)
    {
        app->camera.position -= glm::normalize(glm::cross(app->camera.front, app->camera.up))
                              * camera_speed;
    }
    if (app->inputs.right)
    {
        app->camera.position += glm::normalize(glm::cross(app->camera.front, app->camera.up))
                              * camera_speed;
    }
    // FPS style, stay on ground, no floating
    app->camera.position.y = default_camera_height;

    float sensitivity = 0.1;
    mouse_xd *= sensitivity;
    mouse_yd *= sensitivity;

    app->camera.yaw += mouse_xd;
    app->camera.pitch += mouse_yd;
    app->camera.pitch = clamp(-89.9f, app->camera.pitch, 89.9f);
    // if (app->camera.pitch > 89.0f) app->camera.pitch = 89.0f;
    // if (app->camera.pitch < -89.0f) app->camera.pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(app->camera.yaw)) * cos(glm::radians(app->camera.pitch));
    front.y = sin(glm::radians(app->camera.pitch));
    front.z = sin(glm::radians(app->camera.yaw)) * cos(glm::radians(app->camera.pitch));
    app->camera.front = glm::normalize(front);

    app->camera.fov += mouse_wheel_y;
    app->camera.fov = clamp(10.0f, app->camera.fov,45.0f);

    return quit;
}


int
main(int argc, char *argv[])
{
    App app = {
        .window_width = 1366,
        .window_height = 768,
        .frag_shader_path = "data/shaders/frag.frag",
        .vert_shader_path = "data/shaders/vert.vert",
        .texture1_path = "data/textures/stone.png",
        .texture2_path = "data/textures/coin.png",
    };
    Clock_init(&app.clock);
    Camera_init(&app.camera);

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

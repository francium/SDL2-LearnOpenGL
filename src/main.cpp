#include <stdio.h>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SDL2/SDL.h>

#include "camera.hpp"
#include "clock.hpp"
#include "entity.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "platform.hpp"
#include "result.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "util.hpp"

#include "data.hpp"


const f32 default_camera_height = 2.0f;
const f32 default_fov = 45.0f;
const u32 point_light_count = 2;


// Prevent camera from jumping due to mouse coming into window.
// There may still be a bit of jumpiness, but less so with these two flags
bool first_motion_input = false;
bool first_mouse_wheel_input = false;


struct DirectionLight
{
    glm::vec3 direction;

    glm::vec3  ambient;
    glm::vec3  diffuse;
    glm::vec3  specular;
};

struct PointLight
{
    glm::vec3  ambient;
    glm::vec3  diffuse;
    glm::vec3  specular;

    f32 constant;
    f32 linear;
    f32 quadratic;
};

struct SpotLight
{
    glm::vec3 direction;

    f32 cut_off;
    f32 outer_cut_off;

    glm::vec3  ambient;
    glm::vec3  diffuse;
    glm::vec3  specular;

    f32 constant;
    f32 linear;
    f32 quadratic;
};

union Light
{
    DirectionLight direction;
    PointLight point;
    SpotLight spot;
};

struct Textures
{
    Texture *diffuse,
            *specular;
};

struct Obj
{
    GLuint vao,
           vbo;
    Shader shader;
    Textures textures;
    glm::vec3 position;
};


struct LightObj
{
    Obj obj;
    Light light;
};

struct WorldLights
{
    LightObj *points;
    LightObj sun;
    LightObj flashlight;
};


internal bool
Obj_init(
    Obj *obj,
    const char *vert_shader_path,
    const char *obj_frag_shader_path,
    Textures textures
)
{
    *obj = {};
    obj->textures = textures;

    init_rendering_data(&obj->vao, &obj->vbo);

    bool ok = Shader_init(
        &obj->shader,
        vert_shader_path,
        obj_frag_shader_path
    );

    return ok;
}


internal void
Obj_destroy(Obj *obj)
{
    glDeleteVertexArrays(1, &obj->vao);
    glDeleteBuffers(1, &obj->vbo);
    Shader_destroy(&obj->shader);
}


struct Inputs
{
    bool forward;
    bool backward;
    bool left;
    bool right;
    bool flashlight_follow;
    bool enable_flashlight;
    bool enable_sun;
    bool enable_point_lights;
};


struct App
{
    u32 window_width;
    u32 window_height;

    SDL_Window *window;
    SDL_GLContext context;

    Clock clock;
    Inputs inputs;
    Camera camera;
    Entity *house;

    const char *frag_shader_path;
    const char *vert_shader_path;
    Shader shader;
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
        fprintf(stderr, "Failed to create GL context\n");
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
    SDL_GL_SetSwapInterval(4); // Use VSYNC

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
cleanup_sdl(App *app)
{
    SDL_GL_DeleteContext(app->context);
    SDL_DestroyWindow(app->window);
    SDL_Quit();
}


internal void
cleanup(App *app)
{
    cleanup_sdl(app);
    app->house->cleanup();
    delete app->house;
}


internal bool
init_objs(App *app)
{
    bool ok;

    ok = Shader_init(
        &app->shader,
        app->vert_shader_path,
        app->frag_shader_path
    );
    if (!ok)
    {
        fprintf(stderr, "Failed to init shader\n");
    }

    app->house = new Entity("data/models/winter-scene/", "Low Poly Winter Scene.obj", &app->shader);

    return ok;
}

internal bool
init(App *app)
{
    if (!init_sdl(app)) return false;
    if (!init_gl(app)) return false;
    if (!init_objs(app)) return false;

    return true;
}


internal void
update(App *app)
{
    glClearColor(0.502f, 0.678f, .996f, 1.0f); // Light sky
    // glClearColor(0.001f, 0.038f, .096f, 1.0f); // Dark sky
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Transforms transforms = {};
    transforms.view = Camera_toViewMatrix(&app->camera);
    transforms.projection = glm::perspective(
        glm::radians(app->camera.fov),
        (f32)app->window_width / (f32)app->window_height,
        0.1f,
        100.0f
    );

    app->house->draw(&transforms);

    SDL_GL_SwapWindow(app->window);
}


/** Return of a `true` indicates program should exit */
internal bool
process_events(App *app)
{
    bool quit = false;
    bool mouse_checked = false;
    f32 mouse_dx = 0.0f;
    f32 mouse_dy = 0.0f;
    f32 mouse_wheel_dy = 0.0f;

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

                // Make sure we don't for another motion event as result of
                // centering mouse
                if (mouse_checked) break;

                mouse_dx = (f32)event.motion.xrel;
                mouse_dy = -(f32)event.motion.yrel;

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

                mouse_wheel_dy = -(f32)event.wheel.y;
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
                if (event.key.keysym.sym == SDLK_f)
                {
                    app->inputs.flashlight_follow = !app->inputs.flashlight_follow;
                }
                if (event.key.keysym.sym == SDLK_1)
                {
                    app->inputs.enable_sun = !app->inputs.enable_sun;
                }
                if (event.key.keysym.sym == SDLK_2)
                {
                    app->inputs.enable_point_lights = !app->inputs.enable_point_lights;
                }
                if (event.key.keysym.sym == SDLK_3)
                {
                    app->inputs.enable_flashlight = !app->inputs.enable_flashlight;
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

    if (app->inputs.forward)
        Camera_process_keyboard(&app->camera, CameraForward, app->clock.dt);
    if (app->inputs.backward)
        Camera_process_keyboard(&app->camera, CameraBackward, app->clock.dt);
    if (app->inputs.left)
        Camera_process_keyboard(&app->camera, CameraLeft, app->clock.dt);
    if (app->inputs.right)
        Camera_process_keyboard(&app->camera, CameraRight, app->clock.dt);

    Camera_process_mouse_motion(&app->camera, mouse_dx, mouse_dy);
    Camera_process_mouse_scroll(&app->camera, mouse_wheel_dy);

    return quit;
}


int
main(int argc, char *argv[])
{
    App app = {};
    app.window_width = 1366,
    app.window_height = 768,
    app.frag_shader_path = "data/shaders/model.frag",
    app.vert_shader_path = "data/shaders/vert.vert",
    Clock_init(&app.clock);
    Camera_init(&app.camera, default_camera_height, default_fov);

    fprintf(stderr, "Initializing...\n");
    if (!init(&app))
    {
        cleanup(&app);
        return 1;
    }

    fprintf(stderr, "Starting...\n");

    int i = 0;

    bool should_run = true;
    while (should_run)
    {
        const bool quit = process_events(&app);
        if (quit) break;

        Clock_tick(&app.clock);
        update(&app);

        if ((++i % 60) == 0)
            printf("FPS=%.1f\n", 1 / app.clock.dt * 1000);
    }

    fprintf(stderr, "Exiting...\n");
    cleanup(&app);

    return 0;
}

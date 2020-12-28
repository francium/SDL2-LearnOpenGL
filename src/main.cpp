#include <stdio.h>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SDL2/SDL.h>

#include "camera.hpp"
#include "clock.hpp"
#include "platform.hpp"
#include "result.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "util.hpp"

#include "data.hpp"


const f32 default_camera_height = 2.0f;
const f32 default_fov = 45.0f;


// Prevent camera from jumping due to mouse coming into window.
// There may still be a bit of jumpiness, but less so with these two flags
bool first_motion_input = false;
bool first_mouse_wheel_input = false;


struct Light
{
    glm::vec3  ambient;
    glm::vec3  diffuse;
    glm::vec3  specular;

    f32 constant;
    f32 linear;
    f32 quadratic;
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
};


struct App
{
    u32            window_width,
                   window_height;
    SDL_Window     *window;
    SDL_GLContext  context;
    const char     *obj_frag_shader_path,
                   *light_frag_shader_path,
                   *vert_shader_path,
                   *texture_grass_path,
                   *texture_stone_path,
                   *texture_grass_spec_path,
                   *texture_stone_spec_path;
    Texture        texture_stone,
                   texture_stone_spec,
                   texture_grass,
                   texture_grass_spec;
    Clock          clock;
    Inputs         inputs;
    Camera         camera;
    Obj            cube,
                   floor;
    LightObj       cube_light;
    Obj            sun;
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
    Obj_destroy(&app->cube);
    Obj_destroy(&app->floor);
    Obj_destroy(&app->cube_light.obj);

    cleanup_sdl(app);
}


internal bool
load_texture(const char *path, Texture *dest)
{
    Result<Texture> r_texture = Texture_load(path);
    if (!r_texture.ok)
    {
        fprintf(stderr, "Failed to load texture: %s\n", path);
        return false;
    }

    *dest = r_texture.value;
    fprintf(stderr, "Loaded texture: %s into %d (%p)\n", path, dest->id, dest);

    return true;
}


internal bool
load_textures(App *app)
{
    if (!load_texture(app->texture_grass_path, &app->texture_grass)) return false;
    if (!load_texture(app->texture_grass_spec_path, &app->texture_grass_spec)) return false;
    if (!load_texture(app->texture_stone_path, &app->texture_stone)) return false;
    if (!load_texture(app->texture_stone_spec_path, &app->texture_stone_spec)) return false;

    return true;
}


internal bool
init_objs(App *app)
{
    bool ok;

    Textures grass_textures = {
        .diffuse = &app->texture_grass,
        .specular = &app->texture_grass_spec,
    };
    ok = Obj_init(
        &app->floor,
        app->vert_shader_path,
        app->obj_frag_shader_path,
        grass_textures
    );
    if (!ok)
    {
        fprintf(stderr, "Failed to init floor\n");
        return false;
    }

    Textures stone_textures = {
        .diffuse = &app->texture_stone,
        .specular = &app->texture_stone_spec,
    };
    ok = Obj_init(
        &app->cube,
        app->vert_shader_path,
        app->obj_frag_shader_path,
        stone_textures
    );
    if (!ok)
    {
        fprintf(stderr, "Failed to init cube\n");
        return false;
    }

    Obj_init(
        &app->cube_light.obj,
        app->vert_shader_path,
        app->light_frag_shader_path,
        {}
    );
    if (!ok)
    {
        fprintf(stderr, "Failed to init cube_light\n");
        return false;
    }
    app->cube_light.light = {
        .ambient = glm::vec3 (0.2f, 0.2f, 0.2f),
        .diffuse = glm::vec3 (0.6f, 0.6f, 0.6f),
        .specular = glm::vec3(0.2f, 0.2f, 0.2f),
        .constant = 1.0f,
        .linear = 0.09f,
        .quadratic = 0.032f,
    };

    Obj_init(
        &app->sun,
        app->vert_shader_path,
        app->light_frag_shader_path,
        {}
    );
    if (!ok)
    {
        fprintf(stderr, "Failed to init sun\n");
        return false;
    }
    app->sun.position = glm::vec3(-50.0f, 50.0f, 50.0f);

    return ok;
}

internal bool
init(App *app)
{
    if (!init_sdl(app)) return false;
    if (!init_gl(app)) return false;
    if (!load_textures(app)) return false;
    if (!init_objs(app)) return false;

    return true;
}


internal void
render_sun(Obj *sun)
{
    Shader_use(&sun->shader);
    glBindVertexArray(sun->vao);

    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, sun->position);
    model_matrix = glm::translate(model_matrix, glm::vec3(-0.5f, -0.5f, -0.5f));
    model_matrix = glm::scale(model_matrix, glm::vec3(2.0f, 2.0f, 2.0f));
    Shader_set_matrix4fv(
        &sun->shader,
        "model_matrix",
        glm::value_ptr(model_matrix)
    );

    glDrawArrays(GL_TRIANGLES, 0, 36);
}


internal void
render_light(LightObj *light)
{
    Shader_use(&light->obj.shader);
    glBindVertexArray(light->obj.vao);

    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, light->obj.position);
    model_matrix = glm::translate(model_matrix, glm::vec3(-0.5f, -0.5f, -0.5f));
    model_matrix = glm::scale(model_matrix, glm::vec3(0.25f, 0.25f, 0.25f));
    Shader_set_matrix4fv(
        &light->obj.shader,
        "model_matrix",
        glm::value_ptr(model_matrix)
    );

    glDrawArrays(GL_TRIANGLES, 0, 36);
}

internal void
render_floor(Obj *floor, Camera *camera, LightObj *light, Obj *sun)
{
    Shader_use(&floor->shader);
    Texture_use(floor->textures.diffuse, GL_TEXTURE0);
    Texture_use(floor->textures.specular, GL_TEXTURE1);
    Shader_seti(&floor->shader, "material.diffuse", 0);
    Shader_seti(&floor->shader, "material.specular", 1);
    glBindVertexArray(floor->vao);

    Shader_setv3(&floor->shader, "light.position", camera->position);
    Shader_setv3(&floor->shader, "light.direction", camera->front);
    Shader_setv3(&floor->shader, "light.ambient", light->light.ambient);
    Shader_setv3(&floor->shader, "light.diffuse", light->light.diffuse);
    Shader_setv3(&floor->shader, "light.specular", light->light.specular);
    Shader_setf(&floor->shader, "light.constant", light->light.constant);
    Shader_setf(&floor->shader, "light.linear", light->light.linear);
    Shader_setf(&floor->shader, "light.quadratic", light->light.quadratic);
    Shader_setf(&floor->shader, "light.cut_off", glm::cos(glm::radians(12.5f)));

    Shader_setv3(&floor->shader, "view_pos", camera->position);

    Shader_setf(&floor->shader, "material.shine", 2.0f);

    i32 half_width = 20;
    i32 half_length = 20;

    for (i32 z = -half_length; z < half_length; z++)
    {
        for (i32 x = -half_width; x < half_width; x++)
        {
            glm::mat4 model_matrix = glm::mat4(1.0f);
            model_matrix = glm::translate(
                model_matrix,
                glm::vec3(1.0f * (f32)x, 0.0f, 1.0f * (f32)z)
            );
            Shader_set_matrix4fv(
                &floor->shader,
                "model_matrix",
                glm::value_ptr(model_matrix)
            );


            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
}


internal void
render_objects(Obj *cube, Camera *camera, LightObj *light, Obj *sun)
{
    Shader_use(&cube->shader);
    Texture_use(cube->textures.diffuse, GL_TEXTURE0);
    Texture_use(cube->textures.specular, GL_TEXTURE1);
    Shader_seti(&cube->shader, "material.diffuse", 0);
    Shader_seti(&cube->shader, "material.specular", 1);
    glBindVertexArray(cube->vao);

    Shader_setv3(&cube->shader, "light.position", camera->position);
    Shader_setv3(&cube->shader, "light.direction", camera->front);
    Shader_setv3(&cube->shader, "light.ambient", light->light.ambient);
    Shader_setv3(&cube->shader, "light.diffuse", light->light.diffuse);
    Shader_setv3(&cube->shader, "light.specular", light->light.specular);
    Shader_setf(&cube->shader, "light.constant", light->light.constant);
    Shader_setf(&cube->shader, "light.linear", light->light.linear);
    Shader_setf(&cube->shader, "light.quadratic", light->light.quadratic);
    Shader_setf(&cube->shader, "light.cut_off", glm::cos(glm::radians(12.5f)));

    Shader_setv3(&cube->shader, "view_pos", camera->position);

    Shader_setf(&cube->shader, "material.shine", 8.0f);

    for (u32 i = 0; i < num_objects; i++)
    {
        glm::mat4 model_matrix = glm::mat4(1.0f);
        model_matrix = glm::translate(model_matrix, obj_positions[i]);
        model_matrix = glm::rotate(
            model_matrix,
            glm::radians((i % 2 == 0) ? 90.0f * i : 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        model_matrix = glm::rotate(
            model_matrix,
            glm::radians((i % 2 == 0) ? 90.0f * i : 0.0f),
            glm::vec3(1.0f, 0.0f, 1.0f)
        );
        Shader_set_matrix4fv(
            &cube->shader,
            "model_matrix",
            glm::value_ptr(model_matrix)
        );

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}


internal void
set_transforms(App *app, Obj *obj)
{
    // TODO: Without doing a Shader_use (glUseProgram), things don't render
    // correctly...why?
    Shader_use(&obj->shader);

    Shader_set_matrix4fv(
        &obj->shader,
        "view_matrix",
        glm::value_ptr(Camera_toViewMatrix(&app->camera))
    );

    glm::mat4 projection_matrix = glm::perspective(
        glm::radians(app->camera.fov),
        (f32)app->window_width / (f32)app->window_height,
        0.1f,
        100.0f
    );
    Shader_set_matrix4fv(
        &obj->shader,
        "projection_matrix",
        glm::value_ptr(projection_matrix)
    );
}


internal void
update(App *app)
{
    f32 t = (f32)app->clock.last_tick.tv_sec + (f32)app->clock.last_tick.tv_nsec / 1e9;
    f32 x = sin(t / 3.0f);
    f32 z = cos(t / 3.0f);
    f32 a = 5;
    f32 b = a - a / 2.0f;

    app->cube_light.obj.position = glm::vec3(a * x + b, 5.0f, a * z + b);

    // glClearColor(0.502f, 0.678f, .996f, 1.0f); // Light sky
    glClearColor(0.002f, 0.078f, .196f, 1.0f); // Dark sky
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    set_transforms(app, &app->floor);
    render_floor(&app->floor, &app->camera, &app->cube_light, &app->sun);

    set_transforms(app, &app->cube);
    render_objects(&app->cube, &app->camera, &app->cube_light, &app->sun);

    set_transforms(app, &app->cube_light.obj);
    render_light(&app->cube_light);

    // set_transforms(app, &app->sun);
    // render_sun(&app->sun);

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
    App app = {
        .window_width = 1366,
        .window_height = 768,
        .obj_frag_shader_path = "data/shaders/obj.frag",
        .light_frag_shader_path = "data/shaders/light.frag",
        .vert_shader_path = "data/shaders/vert.vert",
        .texture_grass_path = "data/textures/low-grass.png",
        .texture_stone_path = "data/textures/low-stone.png",
        .texture_grass_spec_path = "data/textures/low-grass.spec.png",
        .texture_stone_spec_path = "data/textures/low-stone.spec.png",
    };
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

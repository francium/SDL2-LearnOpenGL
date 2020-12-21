#include <stdio.h>

#include <glad/glad.h>
#include <SDL2/SDL.h>


SDL_Window    *m_window;
SDL_GLContext  m_context;
GLuint         m_vao, m_vbo, m_ebo, m_tex;
GLuint         m_vert_shader;
GLuint         m_frag_shader;
GLuint         m_shader_prog;


int initialize()
{
    // Initialize SDL Video
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL video\n");
        return 1;
    }

    // Create main window
    m_window = SDL_CreateWindow(
        "SDL App",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640, 480,
        SDL_WINDOW_OPENGL);
    if (m_window == NULL) {
        fprintf(stderr, "Failed to create main window\n");
        SDL_Quit();
        return 1;
    }

    // Initialize rendering context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE
    );

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    m_context = SDL_GL_CreateContext(m_window);
    if (m_context == NULL) {
        fprintf(stderr, "Failed to create GL context\n");
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        return 1;
    }

    SDL_GL_SetSwapInterval(1); // Use VSYNC

    int gladInitRes = gladLoadGL();
    if (!gladInitRes) {
        fprintf(stderr, "Unable to initialize glad\n");
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        return 0;
    }

    return 0;
}


int Cleanup()
{
    glUseProgram(0);
    glDisableVertexAttribArray(0);
    glDetachShader(m_shader_prog, m_vert_shader);
    glDetachShader(m_shader_prog, m_frag_shader);
    glDeleteProgram(m_shader_prog);
    glDeleteShader(m_vert_shader);
    glDeleteShader(m_frag_shader);
    glDeleteTextures(1, &m_tex);
    glDeleteBuffers(1, &m_ebo);
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
    SDL_GL_DeleteContext(m_context);
    SDL_DestroyWindow(m_window);
    SDL_Quit();

    return 0;
}


int update()
{
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    SDL_GL_SwapWindow(m_window);

    return 0;
}


int main(int argc, char *argv[])
{
    int should_run;

    printf("Initializing...\n");
    if (initialize())
    {
        return 1;
    }

    printf("Running...\n");
    for (should_run = 1; should_run; )
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

        update();
    }

    printf("Exiting...\n");
    Cleanup();
    return 0;
}

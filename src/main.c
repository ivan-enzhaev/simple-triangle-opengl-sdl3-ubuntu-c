#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <glad/glad.h>
#include <stdio.h>

static SDL_Window *window = NULL;
static SDL_GLContext gl_context = NULL;

static GLuint shader_program = 0;
static GLuint vao = 0;
static GLuint vbo = 0;

static const int WIN_WIDTH = 280;
static const int WIN_HEIGHT = 280;

static const char *vertex_shader_source =
    "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "}\n";

static const char *fragment_shader_source =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(1.0, 0.5, 0.0, 1.0); // Orange color\n"
    "}\n";

static GLuint CompileShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char info_log[512];
        glGetShaderInfoLog(shader, sizeof(info_log), NULL, info_log);
        printf("SHADER COMPILATION ERROR:\n%s\n", info_log);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

static GLuint CreateShaderProgram(const char *vs_src, const char *fs_src)
{
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vs_src);
    if (!vs)
        return 0;

    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fs_src);
    if (!fs)
    {
        glDeleteShader(vs);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char info_log[512];
        glGetProgramInfoLog(program, sizeof(info_log), NULL, info_log);
        printf("PROGRAM LINKING ERROR:\n%s\n", info_log);
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    (void)appstate;
    (void)argc;
    (void)argv;

    SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "60");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window = SDL_CreateWindow("OpenGL 3.3 Triangle", WIN_WIDTH, WIN_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context)
    {
        printf("SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_GL_MakeCurrent(window, gl_context))
    {
        printf("SDL_GL_MakeCurrent failed: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_GL_SetSwapInterval(0))
    {
        printf("SDL_GL_SetSwapInterval failed: %s\n", SDL_GetError());
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        return SDL_APP_FAILURE;
    }

    printf("OpenGL Context ready: %s\n", (const char *)glGetString(GL_VERSION));

    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window, &w, &h);
    if (w > 0 && h > 0)
    {
        glViewport(0, 0, w, h);
    }

    shader_program = CreateShaderProgram(vertex_shader_source, fragment_shader_source);
    if (!shader_program)
        return SDL_APP_FAILURE;

    const float vertices[] = {
        0.0f, 0.5f,
        -0.5f, -0.5f,
        0.5f, -0.5f
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    (void)appstate;

    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
    {
        if (event->window.data1 > 0 && event->window.data2 > 0)
        {
            glViewport(0, 0, event->window.data1, event->window.data2);
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    (void)appstate;

    if (!gl_context)
    {
        return SDL_APP_CONTINUE;
    }

    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
    {
        SDL_Delay(16);
        return SDL_APP_CONTINUE;
    }

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    SDL_GL_SwapWindow(window);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    (void)appstate;
    (void)result;

    if (gl_context)
    {
        if (vao)
            glDeleteVertexArrays(1, &vao);
        if (vbo)
            glDeleteBuffers(1, &vbo);
        if (shader_program)
            glDeleteProgram(shader_program);

        SDL_GL_DestroyContext(gl_context);
    }

    if (window)
        SDL_DestroyWindow(window);
    SDL_Quit();
}

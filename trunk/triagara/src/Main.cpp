#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <SDL.h>
#include "glew.h"
#include "Timer.h"

const int triangleCount = 65536;
const float runFor = 5.0f;

void runTest() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 1, 0, -1, 1);

    Timer timer;
    Uint64 triangles = 0;

    glBegin(GL_TRIANGLES);
    while (timer.elapsed() < runFor) {
        glVertex2f(0, 0);
        glVertex2f(0, 0);
        glVertex2f(0, 0);
        ++triangles;
    }
    glEnd();
    glFinish();

    std::cout << "Immediate: " << Uint64(triangles / runFor) << " tri/s" << std::endl;

    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < triangleCount; ++i) {
        glVertex2f(0, 0);
        glVertex2f(0, 0);
        glVertex2f(0, 0);
    }
    glEnd();
    glEndList();
    glDeleteLists(list, 1);

    timer.step();
    triangles = 0;
    while (timer.elapsed() < runFor) {
        glCallList(list);
        triangles += triangleCount;
    }
    glFinish();

    std::cout << "Display List: " << Uint64(triangles / runFor) << " tri/s" << std::endl;

    std::vector<float> vertexArray(3 * triangleCount);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &vertexArray[0]);

    timer.step();
    triangles = 0;
    while (timer.elapsed() < runFor) {
        glDrawArrays(GL_TRIANGLES, 0, triangleCount);
        triangles += triangleCount;
    }
    glFinish();

    glDisableClientState(GL_VERTEX_ARRAY);

    std::cout << "Vertex Arrays: " << Uint64(triangles / runFor) << " tri/s" << std::endl;

    if (GLEW_EXT_compiled_vertex_array) {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, &vertexArray[0]);
        glLockArraysEXT(0, triangleCount);

        timer.step();
        triangles = 0;
        while (timer.elapsed() < runFor) {
            glDrawArrays(GL_TRIANGLES, 0, triangleCount);
            triangles += triangleCount;
        }
        glFinish();

        glUnlockArraysEXT();
        glDisableClientState(GL_VERTEX_ARRAY);

        std::cout << "Compiled Vertex Arrays: " << Uint64(triangles / runFor) << " tri/s" << std::endl;
    } else {
        std::cout << "Compiled Vertex Array extension not found." << std::endl;
    }

    if (GLEW_ARB_vertex_buffer_object) {
        GLuint buffer;
        glGenBuffersARB(1, &buffer);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer);

        glBufferDataARB(
            GL_ARRAY_BUFFER_ARB,
            vertexArray.size() * sizeof(vertexArray[0]),
            &vertexArray[0],
            GL_STATIC_DRAW_ARB);

        glEnableClientState(GL_VERTEX_ARRAY);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer);
        glVertexPointer(3, GL_FLOAT, 0, NULL);

        timer.step();
        triangles = 0;
        while (timer.elapsed() < runFor) {
            glDrawArrays(GL_TRIANGLES, 0, triangleCount);
            triangles += triangleCount;
        }
        glFinish();

        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        glDeleteBuffersARB(1, &buffer);
        std::cout << "Vertex Buffer Objects: " << Uint64(triangles / runFor) << " tri/s" << std::endl;
    } else {
        std::cout << "Vertex Buffer Object extension not found." << std::endl;
    }

    timer.step();
    SDL_GL_SwapBuffers();

    std::cout << "Flipping: " << timer.step() << " s" << std::endl;
}


/**
 * SDL_Quit could have a different calling convention, since it comes
 * from another library.
 */
void quitSDL() {
    SDL_Quit();
}

void throwSDLError(const std::string& prefix) {
    throw std::runtime_error(prefix + ": " + SDL_GetError());
}

void initializeSDL(int initflags) {
    if (SDL_Init(initflags) < 0) {
        throwSDLError("SDL initialization failed");
    }
    atexit(quitSDL);
}

void throwGLEWError(const std::string& prefix, GLenum error) {
    throw std::runtime_error(
        prefix + ": " +
        reinterpret_cast<const char*>(glewGetErrorString(error)));
}

int main(int argc, char** argv) {
    initializeSDL(SDL_INIT_NOPARACHUTE | SDL_INIT_VIDEO | SDL_INIT_TIMER);
    
    // define our minimum requirements for the GL window
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    int mode = SDL_OPENGL | SDL_ANYFORMAT;
    bool fullscreen = false;
    if (fullscreen) {
        mode |= SDL_FULLSCREEN;
    }

    // If we set this before opening the window, the window has
    // this caption.
    SDL_WM_SetCaption("Triagara", 0);

    /**
     * @note SDL does silly things when the created window is larger
     * than the desktop.
     */
    const int width  = 1024;
    const int height = 768;
    if (!SDL_SetVideoMode(width, height, 32, mode)) {
        throwSDLError("Setting video mode failed");
    }

    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        throwGLEWError("Initializing GLEW failed", glewError);
    }

    SDL_ShowCursor(SDL_DISABLE);

    runTest();
}


#if defined(WIN32) && !defined(_CONSOLE)

    #include <windows.h>

    #ifdef __CYGWIN__
    extern "C" int __argc;
    extern "C" char* __argv[];
    #endif

    int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
        return main(__argc, __argv);
    }

#endif

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <SDL.h>
#include "glew.h"
#include "Timer.h"


void runTest() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 1, 0, -1, 1);

    Timer timer;
    Uint64 triangles = 0;
    const float runFor = 5.0f;

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

    const int triangleCount = 1024;
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

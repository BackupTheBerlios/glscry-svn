#include <iostream>
#include <stdexcept>
#include <string>
#include <SDL.h>
#include <SDL_opengl.h>


void runTest() {
    Uint32 now = SDL_GetTicks();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 1, 0, -1, 1);

    glBegin(GL_TRIANGLES);
    for (Uint32 i = 0; i < 1000000; ++i) {
        glVertex2f(0, 0);
        glVertex2f(0, 0);
        glVertex2f(0, 0);
    }
    glEnd();

    Uint32 step1 = SDL_GetTicks();
    std::cout << "Rendering: " << step1 - now << " ms" << std::endl;

    SDL_GL_SwapBuffers();

    Uint32 step2 = SDL_GetTicks();
    std::cout << "Flipping: " << step2 - step1 << " ms" << std::endl;
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
    SDL_WM_SetCaption("Empyrean", 0);

    /**
     * @note SDL does silly things when the created window is larger
     * than the desktop.
     */
    const int width  = 1024;
    const int height = 768;
    if (!SDL_SetVideoMode(width, height, 32, mode)) {
        throwSDLError("Setting video mode failed");
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

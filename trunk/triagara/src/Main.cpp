#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <SDL.h>
#include "glew.h"
#include "Timer.h"
using namespace std;

void output(ostream& os, const char* name, const char* label, Uint64 value) {
    os << value << " ";
    cout << name << ": " << value << " " << label << endl;
}

void pumpMessages() {
    SDL_Event event;
    int result = SDL_PollEvent(&event);
    while (result == 1) {
        if (event.type == SDL_QUIT) {
            exit(EXIT_FAILURE);
        }

        result = SDL_PollEvent(&event);
    }
}

void flip() {
    SDL_GL_SwapBuffers();
}

void betweenTests() {
    glClear(GL_COLOR_BUFFER_BIT);
    pumpMessages();
    flip();
}

void runTest(ostream& os, int triangleCount, float runFor = 0.2f) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1024, 768, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    betweenTests();

    {
        Uint64 triangles = 0;
        Timer timer;
        while (timer.elapsed() < runFor) {
            glBegin(GL_TRIANGLES);
            for (int i = 0; i < triangleCount; ++i) {
                glVertex2f(0, 0);
                glVertex2f(1, 0);
                glVertex2f(1, 1);
            }
            glEnd();

            triangles += triangleCount;
        }
        glFinish();

        output(os, "Immediate", "tri/s", Uint64(triangles / timer.elapsed()));
    }

    betweenTests();

    {
        GLuint list = glGenLists(1);
        glNewList(list, GL_COMPILE);
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < triangleCount; ++i) {
            glVertex2f(0, 0);
            glVertex2f(1, 0);
            glVertex2f(1, 1);
        }
        glEnd();
        glEndList();

        Uint64 triangles = 0;

        Timer timer;
        while (timer.elapsed() < runFor) {
            glCallList(list);
            triangles += triangleCount;
        }
        glFinish();

        output(os, "Display List", "tri/s", Uint64(triangles / timer.elapsed()));

        glDeleteLists(list, 1);
    }

    betweenTests();

/*
    vector<float> vertexArray(2 * 3 * triangleCount);
    for (int i = 0; i < triangleCount; ++i) {
        vertexArray[i * 6 + 0 + 0] = 0;
        vertexArray[i * 6 + 0 + 1] = 0;
        vertexArray[i * 6 + 1 + 0] = 1;
        vertexArray[i * 6 + 1 + 1] = 0;
        vertexArray[i * 6 + 2 + 0] = 1;
        vertexArray[i * 6 + 2 + 1] = 1;
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, &vertexArray[0]);

    timer.step();
    triangles = 0;
    while (timer.elapsed() < runFor) {
        glDrawArrays(GL_TRIANGLES, 0, triangleCount * 3);
        triangles += triangleCount;
    }
    glFinish();

    glDisableClientState(GL_VERTEX_ARRAY);

    output(os, "Vertex Arrays", "tri/s", Uint64(triangles / runFor));

    if (GLEW_EXT_compiled_vertex_array) {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, &vertexArray[0]);
        glLockArraysEXT(0, triangleCount);

        timer.step();
        triangles = 0;
        while (timer.elapsed() < runFor) {
            glDrawArrays(GL_TRIANGLES, 0, triangleCount * 3);
            triangles += triangleCount;
        }
        glFinish();

        glUnlockArraysEXT();
        glDisableClientState(GL_VERTEX_ARRAY);

        output(os, "Compiled Vertex Arrays", "tri/s", Uint64(triangles / runFor));
    } else {
        output(os, "Compiled Vertex Arrays", "tri/s", 0);
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
        glVertexPointer(2, GL_FLOAT, 0, NULL);

        timer.step();
        triangles = 0;
        while (timer.elapsed() < runFor) {
            glDrawArrays(GL_TRIANGLES, 0, triangleCount * 3);
            triangles += triangleCount;
        }
        glFinish();

        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        glDeleteBuffersARB(1, &buffer);
        output(os, "Vertex Buffer Objects", "tri/s", Uint64(triangles / runFor));
    } else {
        output(os, "Vertex Buffer Objects", "tri/s", 0);
    }
*/

    os << endl;
}


/**
 * SDL_Quit could have a different calling convention, since it comes
 * from another library.
 */
void quitSDL() {
    SDL_Quit();
}

void throwSDLError(const string& prefix) {
    throw runtime_error(prefix + ": " + SDL_GetError());
}

void initializeSDL(int initflags) {
    if (SDL_Init(initflags) < 0) {
        throwSDLError("SDL initialization failed");
    }
    atexit(quitSDL);
}

void throwGLEWError(const string& prefix, GLenum error) {
    throw runtime_error(
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

    ofstream of("results.data");
    for (int i = 0; i <= 24; ++i) {
        cout << "Running with triangle count of " << (1 << i) << endl;
        runTest(of, 1 << i);
    }
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

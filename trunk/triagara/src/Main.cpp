#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <SDL.h>
#include "glew.h"
#include "GeometryGenerator.h"
#include "GLUtility.h"
#include "Timer.h"
using namespace std;

const int    WIDTH  = 1024;
const int    HEIGHT = 768;
const double NEAR   = -1.0;
const double FAR    = 1.0;

void output(ostream& os, const char* name, const char* label, Uint64 value) {
    os << value << " ";
    cout << "    " << name << ": " << value << " " << label << endl;
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

struct Test {
    virtual ~Test() { }

    virtual bool supported() {
        return true;
    }

    virtual const char* name() = 0;
    virtual const char* units() { return "tri/s"; }

    void generateTriangles(const GeometryGenerator& gen, size_t triangleCount) {
        gen.generate(_triangleBuffer, triangleCount);
    }

    virtual void setup()    { }
    virtual void teardown() { }
    virtual Uint64 run(float runFor) = 0;

protected:
    const vector<Triangle>& getTriangleBuffer() const {
        return _triangleBuffer;
    }

private:
    vector<Triangle> _triangleBuffer;
};

struct ImmediateTest : public Test {
    const char* name() { return "Immediate"; }

    Uint64 run(float runFor) {
        const vector<Triangle>& buffer = getTriangleBuffer();

        Uint64 triangles = 0;
        Timer timer;
        while (timer.elapsed() < runFor) {
            glBegin(GL_TRIANGLES);
            for (size_t i = 0; i < buffer.size(); ++i) {
                glVertex(buffer[i].v1);
                glVertex(buffer[i].v2);
                glVertex(buffer[i].v3);
            }
            glEnd();

            triangles += buffer.size();

            pumpMessages();
        }
        glFinish();

        return Uint64(triangles / timer.elapsed());
    }
};

struct DisplayListTest : public Test {
    const char* name() { return "Display List"; }

    Uint64 run(float runFor) {
        const vector<Triangle>& buffer = getTriangleBuffer();

        GLuint list = glGenLists(1);
        glNewList(list, GL_COMPILE);
        glBegin(GL_TRIANGLES);
        for (size_t i = 0; i < buffer.size(); ++i) {
            glVertex(buffer[i].v1);
            glVertex(buffer[i].v2);
            glVertex(buffer[i].v3);
        }
        glEnd();
        glEndList();

        Uint64 triangles = 0;

        Timer timer;
        while (timer.elapsed() < runFor) {
            glCallList(list);
            triangles += buffer.size();

            pumpMessages();
        }
        glFinish();

        return Uint64(triangles / timer.elapsed());

        glDeleteLists(list, 1);
    }
};

struct VertexArrayTest : public Test {
    const char* name() { return "Vertex Array"; }

    Uint64 run(float runFor) {
        const vector<Triangle>& buffer = getTriangleBuffer();

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(buffer);

        Timer timer;
        Uint64 triangles = 0;
        while (timer.elapsed() < runFor) {
            glDrawArrays(GL_TRIANGLES, 0, buffer.size() * 3);
            triangles += buffer.size();

            pumpMessages();
        }
        glFinish();

        return Uint64(triangles / timer.elapsed());

        glDisableClientState(GL_VERTEX_ARRAY);
    }
};

struct CompiledVertexArrayTest : public Test {
    bool supported() {
        return GLEW_EXT_compiled_vertex_array;
    }

    const char* name() { return "Compiled Vertex Array"; }

    Uint64 run(float runFor) {
        const vector<Triangle>& buffer = getTriangleBuffer();

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(buffer);
        glLockArraysEXT(0, buffer.size());

        Timer timer;
        Uint64 triangles = 0;
        while (timer.elapsed() < runFor) {
            glDrawArrays(GL_TRIANGLES, 0, buffer.size() * 3);
            triangles += buffer.size();
        }
        glFinish();

        return Uint64(triangles / timer.elapsed());

        glUnlockArraysEXT();
        glDisableClientState(GL_VERTEX_ARRAY);
    }
};


struct VertexBufferObjectTest : public Test {
    bool supported() {
        return GLEW_ARB_vertex_buffer_object;
    }

    const char* name() { return "Vertex Buffer Object"; }

    Uint64 run(float runFor) {
        const vector<Triangle>& vertexArray = getTriangleBuffer();

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
        glVertexPointer(Triangle::Vector::Size,
                        GLTypeConstant<Triangle::Vector::DataType>::Result,
                        0, NULL);

        Timer timer;
        Uint64 triangles = 0;
        while (timer.elapsed() < runFor) {
            glDrawArrays(GL_TRIANGLES, 0, vertexArray.size() * 3);
            triangles += vertexArray.size();
            pumpMessages();
        }
        glFinish();

        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        glDeleteBuffersARB(1, &buffer);
        return Uint64(triangles / timer.elapsed());
    }
};

void runTest(Test* test, std::ostream& os, const GeometryGenerator& gen,
             int triangleCount, float runFor) {
    if (test->supported()) {
        test->generateTriangles(gen, triangleCount);
        test->setup();
        Uint64 result = test->run(runFor);
        test->teardown();
        output(os, test->name(), test->units(), result);
    } else {
        output(os, test->name(), test->units(), 0);
    }
}

template<typename T>
void delete_function(T* t) {
    delete t;
}

void runTests(ostream& os, const GeometryGenerator& gen, int triangleCount,
              float runFor = 0.1f) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Is this right?
    // glOrtho(0.5, WIDTH - 0.5, HEIGHT - 0.5, 0.5, NEAR, FAR);
    glOrtho(0, WIDTH, HEIGHT, 0, NEAR, FAR);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    std::vector<Test*> testList;
    testList.push_back(new ImmediateTest);
    testList.push_back(new DisplayListTest);
    testList.push_back(new VertexArrayTest);
    testList.push_back(new CompiledVertexArrayTest);
    testList.push_back(new VertexBufferObjectTest);

    for (size_t i = 0; i < testList.size(); ++i) {
        betweenTests();
        runTest(testList[i], os, gen, triangleCount, runFor);
    }
    
    for_each(testList.begin(), testList.end(), delete_function<Test>);

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


void runTestRange(const std::string& filename, const GeometryGenerator& gen,
                  int beginRange, int endRange) {
    ofstream of(filename.c_str());
    if (!of) {
        throw std::runtime_error("Could not open " + filename);
    }

    for (int i = beginRange; i <= endRange; ++i) {
        cout << "Running tests with batch size of "
             << (1 << i) << " triangles" << endl;
        runTests(of, gen, 1 << i);
    }
}


void run() {
    initializeSDL(SDL_INIT_NOPARACHUTE | SDL_INIT_VIDEO | SDL_INIT_TIMER);
    
    // define our minimum requirements for the GL window
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    int mode = SDL_OPENGL | SDL_ANYFORMAT;
    const bool fullscreen = false;
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
    if (!SDL_SetVideoMode(WIDTH, HEIGHT, 32, mode)) {
        throwSDLError("Setting video mode failed");
    }

    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        throwGLEWError("Initializing GLEW failed", glewError);
    }

    SDL_ShowCursor(SDL_DISABLE);

    runTestRange("zeroes.data",          Zeroes().get(),         0, 14);
    runTestRange("small_triangles.data", SmallTriangles().get(), 0, 14);
}

int main(int /*argc*/, char** /*argv*/) {
    try {
        run();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e) {
        cout << "Exception: " << e.what() << endl;
    }
#if 0
    catch (...) {
        // This catches _everything_, including null pointer accesses
        // and div by zero.
    }
#endif
    return EXIT_FAILURE;
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

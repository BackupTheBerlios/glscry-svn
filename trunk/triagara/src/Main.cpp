#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <SDL.h>
#include "Context.h"
#include "glew.h"
#include "GeometryGenerator.h"
#include "GeometryTest.h"
#include "GLUtility.h"
#include "Test.h"
#include "TextureUploadTest.h"
#include "Timer.h"

#include "CopyPixelTest.h"
#include "DrawPixelTest.h"
#include "ReadPixelTest.h"

using namespace std;


TRIAGARA_BEGIN_NAMESPACE

const int    WIDTH  = 1024;
const int    HEIGHT = 768;
const double NEAR   = -1.0;
const double FAR    = 1.0;

void output(ostream& os, const char* name, const char* label, Uint64 value) {
    os << value << " ";
    cout << "    " << name << ": " << value << " " << label << endl;
}

void flip() {
    SDL_GL_SwapBuffers();
}

void betweenTests() {
    glClear(GL_COLOR_BUFFER_BIT);
    pumpMessages();
    flip();
}

class ImmediateTest : public GeometryTest {
public:
    const char* name() const { return "Immediate"; }

    Uint64 iterate() {
        const vector<Triangle>& buffer = getTriangleBuffer();

        glBegin(GL_TRIANGLES);
        for (size_t i = 0; i < buffer.size(); ++i) {
            glVertex(buffer[i].v1);
            glVertex(buffer[i].v2);
            glVertex(buffer[i].v3);
        }
        glEnd();

        return buffer.size();
    }
};

class DisplayListTest : public GeometryTest {
    GLuint _list;

public:
    const char* name() const { return "Display List"; }

    void setup() {
        const vector<Triangle>& buffer = getTriangleBuffer();

        _list = glGenLists(1);
        glNewList(_list, GL_COMPILE);
        glBegin(GL_TRIANGLES);
        for (size_t i = 0; i < buffer.size(); ++i) {
            glVertex(buffer[i].v1);
            glVertex(buffer[i].v2);
            glVertex(buffer[i].v3);
        }
        glEnd();
        glEndList();
    }

    Uint64 iterate() {
        const vector<Triangle>& buffer = getTriangleBuffer();

        glCallList(_list);
        return buffer.size();
    }

    void teardown() {
        glDeleteLists(_list, 1);
    }
};

class VertexArrayTest : public GeometryTest {
public:
    const char* name() const { return "Vertex Array"; }

    void setup() {
        const vector<Triangle>& buffer = getTriangleBuffer();
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(buffer);
    }

    Uint64 iterate() {
        const vector<Triangle>& buffer = getTriangleBuffer();
        glDrawArrays(GL_TRIANGLES, 0, buffer.size() * 3);
        return buffer.size();
    }

    void teardown() {
        glDisableClientState(GL_VERTEX_ARRAY);
    }
};

class CompiledVertexArrayTest : public GeometryTest {
public:
    const char* name() const { return "Compiled Vertex Array"; }

    bool supported() const {
        return GLEW_EXT_compiled_vertex_array;
    }

    void setup() {
        const vector<Triangle>& buffer = getTriangleBuffer();

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(buffer);
        glLockArraysEXT(0, buffer.size());
    }

    Uint64 iterate() {
        const vector<Triangle>& buffer = getTriangleBuffer();

        glDrawArrays(GL_TRIANGLES, 0, buffer.size() * 3);
        return buffer.size();
    }

    void teardown() {
        glUnlockArraysEXT();
        glDisableClientState(GL_VERTEX_ARRAY);
    }
};


class VertexBufferObjectTest : public GeometryTest {
    GLuint _buffer;

public:
    const char* name() const { return "Vertex Buffer Object"; }

    bool supported() const {
        return GLEW_ARB_vertex_buffer_object;
    }

    void setup() {
        const vector<Triangle>& vertexArray = getTriangleBuffer();

        glGenBuffersARB(1, &_buffer);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, _buffer);

        glBufferDataARB(
            GL_ARRAY_BUFFER_ARB,
            vertexArray.size() * sizeof(vertexArray[0]),
            &vertexArray[0],
            GL_STATIC_DRAW_ARB);

        glEnableClientState(GL_VERTEX_ARRAY);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, _buffer);
        glVertexPointer(Triangle::Vector::Size,
                        GLTypeConstant<Triangle::Vector::DataType>::Result,
                        0, NULL);
    }

    Uint64 iterate() {
        const vector<Triangle>& vertexArray = getTriangleBuffer();
        glDrawArrays(GL_TRIANGLES, 0, vertexArray.size() * 3);
        return vertexArray.size();
    }

    void teardown() {
        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        glDeleteBuffersARB(1, &_buffer);
    }
};

void runTest(GeometryTest* test, std::ostream& os,
             const GeometryGenerator& gen,
             int triangleCount, float runFor) {
    if (test->supported()) {
        test->generateTriangles(gen, triangleCount);
        Uint64 result = test->run(runFor);
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

    std::vector<GeometryTest*> testList;
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


void runPixelTests(const std::string& filename, float runFor = 1.0f) {
    ofstream of(filename.c_str());
    if (!of) {
        throw std::runtime_error("Could not open " + filename);
    }
    ostream& os = of;

    std::vector<Test*> testList;
    testList.push_back(new CopyPixelTest);
    testList.push_back(new DrawPixelTest);
    testList.push_back(new ReadPixelTest);

    for (size_t i = 0; i < testList.size(); ++i) {
        betweenTests();

        Test* test = testList[i];
        if (test->supported()) {
            Uint64 result = test->run(runFor);
            output(os, test->name(), test->units(), result);
        } else {
            output(os, test->name(), test->units(), 0);
        }
    }
    
    for_each(testList.begin(), testList.end(), delete_function<Test>);

    os << endl;    
}


void runTexUploadTests(const std::string& filename, float runFor = 1.0f) {
    ofstream of(filename.c_str());
    if (!of) {
        throw std::runtime_error("Could not open " + filename);
    }
    ostream& os = of;

    std::vector<Test*> testList;
    testList.push_back(new TextureUploadTest);

    for (size_t i = 0; i < testList.size(); ++i) {
        betweenTests();

        Test* test = testList[i];
        if (test->supported()) {
            Uint64 result = test->run(runFor);
            output(os, test->name(), test->units(), result);
        } else {
            output(os, test->name(), test->units(), 0);
        }
    }
    
    for_each(testList.begin(), testList.end(), delete_function<Test>);

    os << endl;    
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

    //runTestRange("zeroes.data",          Zeroes(),         0, 14);
    //runTestRange("small_triangles.data", SmallTriangles(), 0, 14);
    //runPixelTests("pixel.data");
    runTexUploadTests("upload.data");
}

TRIAGARA_END_NAMESPACE


int main(int /*argc*/, char** /*argv*/) {
    try {
        triagara::run();
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
    extern "C" __declspec(dllimport) int __argc;
    extern "C" __declspec(dllimport) char* __argv[];
    #endif

    int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
        return main(__argc, __argv);
    }

#endif

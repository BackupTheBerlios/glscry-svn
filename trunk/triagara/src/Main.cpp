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

#include "ImmediateTest.h"
#include "DisplayListTest.h"
#include "VertexArrayTest.h"
#include "CompiledVertexArrayTest.h"
#include "VertexBufferObjectTest.h"

#include "CopyPixelTest.h"
#include "DrawPixelTest.h"
#include "ReadPixelTest.h"

#include "TextureUploadTest.h"


TRIAGARA_BEGIN_NAMESPACE

const int    WIDTH  = 1024;
const int    HEIGHT = 768;
const double NEAR   = -1.0;
const double FAR    = 1.0;

void output(std::ostream& os, Test* test, const ResultSet& results,
            const std::string& depVar) {
    std::vector<ResultDesc> descs;
    test->getResultDescs(descs);
    assert(descs.size() == results.size());

    int resultIndex = -1;
    for (size_t i = 0; i < descs.size(); ++i) {
        if (descs[i].name == depVar) {
            resultIndex = i;
            break;
        }
    }
    if (resultIndex == -1) {
        throw std::runtime_error("Test has no such result");
    }
    
    os << results[resultIndex] << " ";

    std::cout << "  " << test->name() << ": ";

    std::cout << descs[resultIndex].name << " = "
              << Uint64(results[resultIndex]) << " "
              << descs[resultIndex].units << std::endl;
}

void flip() {
    SDL_GL_SwapBuffers();
}

void betweenTests() {
    glClear(GL_COLOR_BUFFER_BIT);
    pumpMessages();
    flip();
}


void runTests(const std::string& filename, std::vector<Test*> testList,
              float runFor, const std::string& depVar) {
    
    std::ofstream of(filename.c_str());
    if (!of) {
        throw std::runtime_error("Could not open " + filename);
    }

    for (size_t i = 0; i < testList.size(); ++i) {
        betweenTests();

        Test* test = testList[i];
        if (test->supported()) {
            ResultSet results = test->run(runFor);
            output(of, test, results, depVar);
        } else {
            // output a zero
        }
    }
    
    std::for_each(testList.begin(), testList.end(), delete_function<Test>);

    of << std::endl;
}


void runTest(GeometryTest* test, std::ostream& os,
             const GeometryGenerator& gen,
             int triangleCount, float runFor) {
    if (test->supported()) {
        test->generateTriangles(gen, triangleCount);
        ResultSet results = test->run(runFor);
        output(os, test, results, "TriangleRate");
    }
}

void runGeometryTests(std::ostream& os, const GeometryGenerator& gen,
                      int triangleCount, float runFor = 0.1f) {
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
    
    std::for_each(testList.begin(), testList.end(), delete_function<Test>);

    os << std::endl;
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


void runTestRange(const std::string& filename, const GeometryGenerator& gen,
                  int beginRange, int endRange) {
    std::ofstream of(filename.c_str());
    if (!of) {
        throw std::runtime_error("Could not open " + filename);
    }

    for (int i = beginRange; i <= endRange; ++i) {
        std::cout << "Running tests with batch size of "
                  << (1 << i) << " triangles" << std::endl;
        runGeometryTests(of, gen, 1 << i);
    }
}


void runPixelTests() {
    std::vector<Test*> testList;
    testList.push_back(new CopyPixelTest);
    testList.push_back(new DrawPixelTest);
    testList.push_back(new ReadPixelTest);

    runTests("pixel.data", testList, 1.0f, "PixelRate");
}


void runTexUploadTests() {
    TextureUploadTest* test1 = new TextureUploadTest;
    TextureUploadTest* test2 = new TextureUploadTest;
    test2->setWidth(512);
    test2->setHeight(512);

    std::vector<Test*> testList;
    testList.push_back(test1);
    testList.push_back(test2);

    runTests("upload.data", testList, 1.0f, "PixelRate");
}


void setProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Is this right?
    glOrtho(0, WIDTH, HEIGHT, 0, NEAR, FAR);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
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

    setProjection();

    SDL_ShowCursor(SDL_DISABLE);

    //runTestRange("zeroes.data",          Zeroes(),         0, 14);
    //runTestRange("small_triangles.data", SmallTriangles(), 0, 14);
    runPixelTests();
    runTexUploadTests();
}

TRIAGARA_END_NAMESPACE


int main(int /*argc*/, char** /*argv*/) {
    try {
        triagara::run();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
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

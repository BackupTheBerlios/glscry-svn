#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <boost/python.hpp>
#include "Context.h"
#include "glew.h"
#include "GeometryGenerator.h"
#include "GeometryTest.h"
#include "GLUtility.h"
#include "Range.h"
#include "Run.h"

#include "DepthState.h"
#include "LightState.h"

#include "ImmediateTest.h"
#include "IndexedGeometryTest.h"
#include "DisplayListTest.h"
#include "VertexArrayTest.h"
#include "CompiledVertexArrayTest.h"
#include "VertexBufferObjectTest.h"

#include "CopyPixelTest.h"
#include "DrawPixelTest.h"
#include "ReadPixelTest.h"

#include "TextureUploadTest.h"


SCRY_BEGIN_NAMESPACE


void testBatchSizes(const std::string& filename, const GeometryGenerator& gen,
                      int beginRange, int endRange) {
    std::vector<Test*> testList;
    testList.push_back(new ImmediateTest(&gen));
    testList.push_back(new DisplayListTest(&gen));
    testList.push_back(new VertexArrayTest(&gen));
    testList.push_back(new CompiledVertexArrayTest(&gen));
    testList.push_back(new VertexBufferObjectTest(&gen));

    Range& range = PowerRange(beginRange, endRange).get();
    runTestsRange(filename, testList, 0.2f, "TriangleRate",
                  "BatchSize", range);
}


void testVertexCache() {
    Zeroes z;

    std::vector<Test*> testList;
    testList.push_back(new IndexedGeometryTest(&z));
    runTestsRange("vcache.data", testList, 1.0f, "TriangleRate",
                  "BatchSize", PowerRange(1, 16).get());
}


void testLights() {
    std::vector<Test*> testList;
    for (size_t i = 0; i < 8; ++i) {
        LightState* light = new LightState;
        for (size_t j = 0; j < i; ++j) {
            light->useLight(j, true);
            light->setAmbient(j, Vec4f(0.5f, 0.5f, 0.5f, 1.0f));
        }
        SmallTriangles gen;
        Test* test = new VertexArrayTest(&gen);
        test->setState(light);
        testList.push_back(test);
    }

    runTests("light.data", testList, 1.0f, "TriangleRate");
}


void testHeirarchialZBuffer() {
    Zeroes genFrontToBack;
    Zeroes genBackToFront;

    GeometryTest* frontToBack = new VertexArrayTest(&genFrontToBack);
    GeometryTest* backToFront = new VertexArrayTest(&genBackToFront);

    std::vector<Test*> testList;
    testList.push_back(frontToBack);
    testList.push_back(backToFront);
    runTests("heir_z.data", testList, 1.0f, "TriangleRate");
}


SCRY_END_NAMESPACE


int main(int /*argc*/, char** /*argv*/) {
    try {
        //scry::run();
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
#ifndef TRIAGARA_COMPILED_VERTEX_ARRAY_TEST
#define TRIAGARA_COMPILED_VERTEX_ARRAY_TEST


#include "GeometryTest.h"


TRIAGARA_BEGIN_NAMESPACE

class CompiledVertexArrayTest : public GeometryTest {
public:
    const char* name() const {
        return "Compiled Vertex Array";
    }

    bool supported() const {
        return GLEW_EXT_compiled_vertex_array;
    }

    void setup() {
        const std::vector<Triangle>& buffer = getTriangleBuffer();

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(buffer);
        glLockArraysEXT(0, buffer.size());
    }

    void iterate(ResultSet& results) {
        const std::vector<Triangle>& buffer = getTriangleBuffer();

        glDrawArrays(GL_TRIANGLES, 0, buffer.size() * 3);
        results[0] += buffer.size();
    }

    void teardown() {
        glUnlockArraysEXT();
        glDisableClientState(GL_VERTEX_ARRAY);
    }
};

TRIAGARA_END_NAMESPACE


#endif
#ifndef SCRY_VERTEX_BUFFER_OBJECT_TEST
#define SCRY_VERTEX_BUFFER_OBJECT_TEST


#include "GeometryTest.h"
#include "GLUtility.h"


SCRY_BEGIN_NAMESPACE

class VertexBufferObjectTest : public GeometryTest {
protected:
    ~VertexBufferObjectTest() { }

public:
    static void bind();

    VertexBufferObjectTest(GeometryGeneratorPtr gen)
        : GeometryTest(gen)
    {
    }

    const char* name() const {
        return "Vertex Buffer Object";
    }

    bool supported() const {
        return GLEW_ARB_vertex_buffer_object != 0;
    }

    void setup() {
        const std::vector<Triangle>& vertexArray = getTriangleBuffer();

        glGenBuffersARB(1, &_buffer);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, _buffer);

        glBufferDataARB(
            GL_ARRAY_BUFFER_ARB,
            vertexArray.size() * sizeof(vertexArray[0]),
            &vertexArray[0],
            GL_STATIC_DRAW_ARB);

        glEnableClientState(GL_VERTEX_ARRAY);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, _buffer);
        ::glVertexPointer(Triangle::Vector::Size,
                          GLTypeConstant<Triangle::Vector::DataType>::Result,
                          0, NULL);
    }

    void iterate(ResultSet& results) {
        const std::vector<Triangle>& vertexArray = getTriangleBuffer();
        glDrawArrays(GL_TRIANGLES, 0, vertexArray.size() * 3);
        results[0] += vertexArray.size();
    }

    void teardown() {
        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        glDeleteBuffersARB(1, &_buffer);
    }

private:
    GLuint _buffer;
};
SCRY_REF_PTR(VertexBufferObjectTest);


SCRY_END_NAMESPACE


#endif

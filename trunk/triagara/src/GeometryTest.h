#ifndef TRIAGARA_GEOMETRY_TEST_H
#define TRIAGARA_GEOMETRY_TEST_H


#include "Base.h"
#include "Test.h"


TRIAGARA_BEGIN_NAMESPACE

class GeometryTest : public Test {
public:
    TRIAGARA_BEGIN_RESULT_DESCS()
        TRIAGARA_RESULT_DESC("TriangleRate", "tri/s")
    TRIAGARA_END_RESULT_DESCS()

    void generateTriangles(const GeometryGenerator& gen, size_t triangleCount) {
        gen.generate(_triangleBuffer, triangleCount);
    }

protected:
    const std::vector<Triangle>& getTriangleBuffer() const {
        return _triangleBuffer;
    }

private:
    std::vector<Triangle> _triangleBuffer;
};

TRIAGARA_END_NAMESPACE


#endif

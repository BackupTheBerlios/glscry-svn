#ifndef TRIAGARA_GEOMETRY_TEST_H
#define TRIAGARA_GEOMETRY_TEST_H


#include "Base.h"
#include "Test.h"


TRIAGARA_BEGIN_NAMESPACE

class GeometryTest : public Test {
public:
    void generateTriangles(const GeometryGenerator& gen, size_t triangleCount) {
        gen.generate(_triangleBuffer, triangleCount);
    }

    const char* units() const { return "tri/s"; }

protected:
    const std::vector<Triangle>& getTriangleBuffer() const {
        return _triangleBuffer;
    }

private:
    std::vector<Triangle> _triangleBuffer;
};

TRIAGARA_END_NAMESPACE


#endif

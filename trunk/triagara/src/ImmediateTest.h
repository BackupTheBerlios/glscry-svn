#ifndef TRIAGARA_IMMEDIATE_TEST_H
#define TRIAGARA_IMMEDIATE_TEST_H


#include "GeometryTest.h"


TRIAGARA_BEGIN_NAMESPACE


class ImmediateTest : public GeometryTest {
public:
    const char* name() const {
        return "Immediate";
    }

    void iterate(ResultSet& results) {
        const std::vector<Triangle>& buffer = getTriangleBuffer();

        glBegin(GL_TRIANGLES);
        for (size_t i = 0; i < buffer.size(); ++i) {
            glVertex(buffer[i].v1);
            glVertex(buffer[i].v2);
            glVertex(buffer[i].v3);
        }
        glEnd();

        results[0] += buffer.size();
    }
};


TRIAGARA_END_NAMESPACE


#endif
#include "VertexBufferObjectTest.h"
using namespace boost::python;


namespace scry {

    void VertexBufferObjectTest::bind() {
        class_<VertexBufferObjectTest, VertexBufferObjectTestPtr,
            bases<GeometryTest>, boost::noncopyable>
            ("VertexBufferObjectTest", no_init)
            .def(init<const char*, GeometryPtr>());

        implicitly_convertible<VertexBufferObjectTestPtr, GeometryTestPtr>();
    }

}
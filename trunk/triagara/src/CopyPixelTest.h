#ifndef TRIAGARA_COPY_PIXEL_TEST_H
#define TRIAGARA_COPY_PIXEL_TEST_H


#include "Base.h"
#include "PixelTransferTest.h"


TRIAGARA_BEGIN_NAMESPACE


class CopyPixelTest : public PixelTransferTest {
public:
    const char* name() const { return "CopyPixelTest"; }

    Uint64 iterate() {
        static const int width = 256;
        static const int height = 256;
        glCopyPixels(0, 0, width, height, GL_COLOR);
        return width * height;
    }
};


TRIAGARA_END_NAMESPACE


#endif
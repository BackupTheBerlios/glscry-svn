#ifndef TRIAGARA_DRAW_PIXEL_TEST_H
#define TRIAGARA_DRAW_PIXEL_TEST_H


#include "Base.h"
#include "PixelTransferTest.h"


TRIAGARA_BEGIN_NAMESPACE


class DrawPixelTest : public PixelTransferTest {
public:
    const char* name() const { return "DrawPixelTest"; }

    Uint64 iterate() {
        static const int width = 256;
        static const int height = 256;
        static char buffer[width * height * 4];
        glDrawPixels(256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
        return width * height;
    }
};


TRIAGARA_END_NAMESPACE


#endif

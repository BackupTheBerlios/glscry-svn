#ifndef TRIAGARA_PIXEL_TRANSFER_TEST
#define TRIAGARA_PIXEL_TRANSFER_TEST


#include "Base.h"
#include "Test.h"


TRIAGARA_BEGIN_NAMESPACE

class PixelTransferTest : public Test {
public:
    const char* units() const { return "pixels/sec"; }
};

TRIAGARA_END_NAMESPACE


#endif

#ifndef TRIAGARA_TEXTURE_UPLOAD_TEST_H
#define TRIAGARA_TEXTURE_UPLOAD_TEST_H


#include "Base.h"
#include "Test.h"


TRIAGARA_BEGIN_NAMESPACE

class TextureUploadTest : public Test {
public:
    TextureUploadTest() {
        _texture = 0;
    }

    virtual const char* name() const {
        return "TextureUploadTest";
    }

    virtual const char* units() const {
        return "texels/sec";
    }

    bool supported() const {
        return true;
    }

    void setup() {
        glGenTextures(1, &_texture);
    }

    Uint64 iterate() {
        // Should add a way to test 1D and 3D textures.
        glBindTexture(GL_TEXTURE_2D, _texture);

        static unsigned char data[256 * 256 * 4];

        // This call really needs to be customizable.
        glTexImage2D(GL_TEXTURE_2D, 0, 4, 256, 256, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, data);
        return sizeof(data);
    }

    void teardown() {
        glDeleteTextures(1, &_texture);
    }

private:
    GLuint _texture;
};

TRIAGARA_END_NAMESPACE


#endif

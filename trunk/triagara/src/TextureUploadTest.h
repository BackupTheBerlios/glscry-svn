#ifndef TRIAGARA_TEXTURE_UPLOAD_TEST_H
#define TRIAGARA_TEXTURE_UPLOAD_TEST_H


#include "Base.h"
#include "Test.h"


TRIAGARA_BEGIN_NAMESPACE

class TextureUploadTest : public Test {
public:
    int getWidth() const       { return _width; }
    void setWidth(int width)   { _width = width; }
    int getHeight() const      { return _height; }
    void setHeight(int height) { _height = height; }

    virtual const char* name() const {
        return "TextureUploadTest";
    }

    bool supported() const {
        return true;
    }

    TRIAGARA_BEGIN_RESULT_DESCS()
      TRIAGARA_RESULT_DESC("PixelRate", "pixels/sec")
      TRIAGARA_RESULT_DESC("ByteRate",  "bytes/sec")
    TRIAGARA_END_RESULT_DESCS()

    void setup() {
        glGenTextures(1, &_texture);
    }

    void iterate(ResultSet& results) {
        // Should add a way to test 1D and 3D textures.
        glBindTexture(GL_TEXTURE_2D, _texture);

        _buffer.resize(_width * _height * 4);

        // This call really needs to be customizable.
        glTexImage2D(GL_TEXTURE_2D, 0, 4, _width, _height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, &_buffer[0]);
        results[0] += _width * _height;
        results[1] += _buffer.size();
    }

    void teardown() {
        glDeleteTextures(1, &_texture);
    }

private:
    Inited<int, 256> _width;
    Inited<int, 256> _height;

    GLuint _texture;
    std::vector<unsigned char> _buffer;
};

TRIAGARA_END_NAMESPACE


#endif

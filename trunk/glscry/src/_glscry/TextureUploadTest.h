/* [Begin Copyright Header]
 *
 * GLScry - OpenGL Performance Analysis Tool
 * Copyright (C) 2004-2005  Iowa State University
 * 
 * This software is licensed under the terms of the GNU Lesser Public
 * License, version 2.1, as published by the Free Software Foundation.
 * See the file COPYRIGHT.txt for details.
 * 
 * Authors:
 *   Chad Austin <aegisk@iastate.edu>
 *   Dirk Reiners <dreiners@iastate.edu>
 *
 * [End Copyright Header] */
#ifndef SCRY_TEXTURE_UPLOAD_TEST_H
#define SCRY_TEXTURE_UPLOAD_TEST_H


#include "GLUtility.h"
#include "Test.h"


namespace scry {

    class TextureUploadTest : public Test {
    protected:
        ~TextureUploadTest() { }

    public:
        static void bind();

        TextureUploadTest(const char* name)
        : Test(name) {
        }

        int getWidth() const       { return _width;  }
        void setWidth(int width)   { _width = width; }

        int getHeight() const      { return _height;   }
        void setHeight(int height) { _height = height; }

        int getType() const    { return _type; }
        void setType(int type) { _type = type; }

        SCRY_BEGIN_RESULT_DESCS()
            SCRY_RESULT_DESC("PixelRate", "pixels/sec")
            SCRY_RESULT_DESC("ByteRate",  "bytes/sec")
        SCRY_END_RESULT_DESCS()

        void setup() {
            glGenTextures(1, &_texture);
        }

        void iterate(ResultValues& results) {
            // Should add a way to test 1D and 3D textures.
            glBindTexture(GL_TEXTURE_2D, _texture);

            _buffer.resize(_width * _height * 4 * getTypeSize());

            // This call really needs to be customizable.
            glTexImage2D(GL_TEXTURE_2D, 0, 4, _width, _height, 0, GL_RGBA,
                         _type, &_buffer[0]);

            results[0] += _width * _height;
            results[1] += _buffer.size();
        }

        void teardown() {
            glDeleteTextures(1, &_texture);
        }

    private:
        int getTypeSize() const {
            return scry::getTypeSize(_type);
        }

        Inited<int, 256>              _width;
        Inited<int, 256>              _height;
        //Inited<int, GL_RGBA>          _format;
        Inited<int, GL_UNSIGNED_BYTE> _type;

        GLuint _texture;
        // This might need to be aligned in some way.  Keep an eye out.
        std::vector<unsigned char> _buffer;
    };
    SCRY_REF_PTR(TextureUploadTest);

}


#endif

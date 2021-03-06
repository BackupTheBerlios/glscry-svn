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
#ifndef SCRY_COVERAGE_H
#define SCRY_COVERAGE_H


#include "OpenGL.h"
#include "SDL.h"


namespace scry {

    Uint64 calculateCoverage(
        GLenum primitiveType,
        GLenum dataType,
        size_t size,         ///< Number of elements in vector.
        size_t vertexCount,  ///< Number of vertices in array.
        const void* data);

}


#endif

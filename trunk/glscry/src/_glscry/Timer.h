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
#ifndef SCRY_TIMER_H
#define SCRY_TIMER_H


#include <SDL.h>


namespace scry {

    class Timer {
    public:
        Timer();

        /// Returns time since creation.
        float elapsed() const;

    private:
        Uint32 _last;
    };

}


#endif

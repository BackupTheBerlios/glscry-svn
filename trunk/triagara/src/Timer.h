#ifndef TIMER_H
#define TIMER_H


#include <SDL.h>


class Timer {
public:
    Timer();

    /// Returns time since creation or last step() in seconds.
    float step();

    /// Returns time since creation or last step() in seconds.
    float elapsed() const;

private:
    Uint32 _last;
};


#endif

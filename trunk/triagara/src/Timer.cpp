#include "Timer.h"


Timer::Timer() {
    _last = SDL_GetTicks();
}


float Timer::step() {
    Uint32 now = SDL_GetTicks();
    float result = (now - _last) / 1000.0f;
    _last = now;
    return result;
}

#pragma once


#include <time.h>

#include "platform.hpp"


struct Clock
{
    f32 dt;
    struct timespec last_tick;
};


internal void
Clock_init(Clock *clock)
{
    clock_gettime(CLOCK_MONOTONIC, &clock->last_tick);
}


internal void
Clock_tick(Clock *clock)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    clock->dt = (f32)(now.tv_sec - clock->last_tick.tv_sec)
              + (f32)(now.tv_nsec - clock->last_tick.tv_nsec) / 1e6;
    // FIXME: For some reason dt becomes negative (now < last_tick or this math
    // above to calculate dt is wrong)
    clock->dt = fmax(0, clock->dt);
    clock->last_tick = now;
}

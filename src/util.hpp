#pragma once


#include "platform.hpp"


internal f32
clamp(f32 min, f32 value, f32 max)
{
    return fmax(min, fmin(max, value));
}

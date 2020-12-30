#pragma once


#include <stdio.h>


internal void
DUMPv3(const char *label, glm::vec3 x)
{
    printf("%s = %.1f, %.1f, %.1f\n", label, x.x, x.y, x.z);
}

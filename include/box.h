#pragma once

#include "vec2.h"

typedef struct Box Box;
struct Box {
    Vec2 pos;
    Vec2 size;
};

extern Vec2 box(F32 x, F32 y, F32 w, F32 h);

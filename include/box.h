#pragma once

#include "vec2.h"

typedef struct Box Box;
struct Box {
    Vec2 pos;
    Vec2 size;
};

extern Vec2 box(float x, float y, float w, float h);

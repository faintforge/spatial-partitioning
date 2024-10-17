#pragma once

#include <stdbool.h>

#include "vec2.h"

typedef struct Box Box;
struct Box {
    Vec2 pos;
    Vec2 size;
};

extern Box box(float x, float y, float w, float h);
extern bool box_eq(Box a, Box b);

extern bool box_overlapp(Box a, Box b);

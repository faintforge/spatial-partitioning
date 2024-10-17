#include "box.h"

#include <math.h>

Box box(float x, float y, float w, float h) {
    return (Box) {
        .pos = vec2(x, y),
        .size = vec2(w, h),
    };
}

bool box_eq(Box a, Box b) {
    float epsilon = 0.0001f;

    return fabsf(b.pos.x - a.pos.x) < epsilon &&
        fabsf(b.pos.y - a.pos.y) < epsilon &&
        fabsf(b.size.x - a.size.x) < epsilon &&
        fabsf(b.size.y - a.size.y) < epsilon;
}

bool box_overlapp(Box a, Box b) {
    return a.pos.x+a.size.x > b.pos.x &&
           a.pos.x < b.pos.x+b.size.x &&
           a.pos.y+a.size.y > b.pos.y &&
           a.pos.y < b.pos.y+b.size.y;
}

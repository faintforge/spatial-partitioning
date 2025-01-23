#pragma once

#include "ds.h"
#include "box.h"

#include <SDL2/SDL.h>

typedef struct Naive Naive;
struct Naive {
    Vec(Box) boxes;
};

extern Naive* naive_new(const void* desc);
extern void naive_free(Naive* data);
extern void naive_insert(Naive* data, Box box);
extern void naive_clear(Naive* data);
extern Vec(Box) naive_query(const Naive* data, Box area);
extern void naive_debug_draw(const Naive* data, SDL_Renderer* renderer);

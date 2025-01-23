#pragma once

#include "box.h"
#include "vec2.h"
#include <stdint.h>
#include "ds.h"
#include <SDL2/SDL.h>

#define SPATIAL_HASH_MAX_BOX_COUNT 128

typedef struct Bucket Bucket;
struct Bucket {
    Box boxes[SPATIAL_HASH_MAX_BOX_COUNT];
    uint32_t box_i;
};

typedef struct SpatialHash SpatialHash;
struct SpatialHash {
    Vec2 cell_size;
    uint32_t map_capacity;
    Bucket *buckets;
};

typedef struct SpatialHashDesc SpatialHashDesc;
struct SpatialHashDesc {
    uint32_t map_capacity;
    Vec2 cell_size;
};

extern SpatialHash* spatial_hash_new(const SpatialHashDesc* desc);
extern void spatial_hash_free(SpatialHash *space);

extern void spatial_hash_insert(SpatialHash *space, Box box);
extern void spatial_hash_clear(SpatialHash *space);

extern Vec(Box) spatial_hash_query(const SpatialHash* space, Box area);

extern void spatial_hash_debug_draw(const SpatialHash* space, SDL_Renderer *renderer);

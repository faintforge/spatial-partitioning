#include "hashing.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

// https://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key
static uint64_t hash_position(int32_t x, int32_t y) {
    uint64_t full = ((uint64_t) x << 32) | y;
    full = (full ^ (full >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    full = (full ^ (full >> 27)) * UINT64_C(0x94d049bb133111eb);
    full = full ^ (full >> 31);
    return full;
}

SpatialHash spatial_hash_new(uint32_t map_capacity, Vec2 cell_size) {
    SpatialHash space = {
        .cell_size = cell_size,
        .map_capacity = map_capacity,
        .buckets = calloc(map_capacity, sizeof(Bucket)),
    };

    return space;
}

void spatial_hash_free(SpatialHash *space) {
    free(space->buckets);
}

void spatial_hash_insert(SpatialHash *space, Box box) {
    Vec2 min = vec2_div(box.pos, space->cell_size);
    min.x = floorf(min.x);
    min.y = floorf(min.y);
    Vec2 max = vec2_div(vec2_add(box.pos, box.size), space->cell_size);
    max.x = ceilf(max.x);
    max.y = ceilf(max.y);

    for (int32_t y = min.y; y < max.y; y++) {
        for (int32_t x = min.x; x < max.x; x++) {
            uint64_t hash = hash_position(x, y);
            uint64_t index = hash % space->map_capacity;
            Bucket *bucket = &space->buckets[index];
            bucket->boxes[bucket->box_i++] = box;
            if (bucket->box_i >= SPATIAL_HASH_MAX_BOX_COUNT) {
                printf("WARN: Max box count exceeded for spatial hash bucket.\n");
                exit(1);
            }
        }
    }
}

void spatial_hash_clear(SpatialHash *space) {
    for (uint32_t i = 0; i < space->map_capacity; i++) {
        space->buckets[i].box_i = 0;
    }
}

Vec(Box) spatial_hash_query(SpatialHash space, Box area) {
    Vec(Box) result = NULL;

    Vec2 min = vec2_div(area.pos, space.cell_size);
    min.x = floorf(min.x);
    min.y = floorf(min.y);
    Vec2 max = vec2_div(vec2_add(area.pos, area.size), space.cell_size);
    max.x = ceilf(max.x);
    max.y = ceilf(max.y);

    for (int32_t y = min.y; y < max.y; y++) {
        for (int32_t x = min.x; x < max.x; x++) {
            uint64_t hash = hash_position(x, y);
            uint64_t index = hash % space.map_capacity;
            Bucket *bucket = &space.buckets[index];
            for (uint32_t i = 0; i < bucket->box_i; i++) {
                vec_push(result, bucket->boxes[i]);
            }
        }
    }

    return result;
}

void spatial_hash_debug_draw(SpatialHash space, uint32_t horizontal_count, uint32_t vertical_count, SDL_Renderer *renderer) {
    Vec2 pos = vec2s(0.0f); 
    for (uint32_t y = 0; y < vertical_count; y++) {
        for (uint32_t x = 0; x < horizontal_count; x++) {
            SDL_Rect rect = {
                .x = pos.x,
                .y = pos.y,
                .w = space.cell_size.x,
                .h = space.cell_size.y,
            };
            SDL_RenderDrawRect(renderer, &rect);
            pos.x += space.cell_size.x;
        }
        pos.x = 0.0f;
        pos.y += space.cell_size.y;
    }
}

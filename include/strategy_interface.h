#pragma once

#include "ds.h"
#include "box.h"
#include "quadtree.h"
#include "grid.h"
#include "hashing.h"
#include "naive.h"

#include <SDL2/SDL.h>

typedef void* (*StrategyNewFunc)(const void* desc);
typedef void (*StrategyFreeFunc)(void* data);
typedef void (*StrategyInsertFunc)(void* data, Box box);
typedef void (*StrategyClearFunc)(void* data);
typedef Vec(Box) (*StrategyQueryFunc)(const void* data, Box area);
typedef void (*StrategyDebugDrawFunc)(const void* data, SDL_Renderer* renderer);

typedef struct Strategy Strategy;
struct Strategy {
    StrategyNewFunc new;
    StrategyFreeFunc free;
    StrategyInsertFunc insert;
    StrategyClearFunc clear;
    StrategyQueryFunc query;
    StrategyDebugDrawFunc debug_draw;
};

static const Strategy STRATEGY_QUADTREE = {
    .new        = (StrategyNewFunc)       quadtree_new,
    .free       = (StrategyFreeFunc)      quadtree_free,
    .insert     = (StrategyInsertFunc)    quadtree_insert,
    .clear      = (StrategyClearFunc)     quadtree_clear,
    .query      = (StrategyQueryFunc)     quadtree_query,
    .debug_draw = (StrategyDebugDrawFunc) quadtree_debug_draw,
};

static const Strategy STRATEGY_GRID = {
    .new        = (StrategyNewFunc)       grid_new,
    .free       = (StrategyFreeFunc)      grid_free,
    .insert     = (StrategyInsertFunc)    grid_insert,
    .clear      = (StrategyClearFunc)     grid_clear,
    .query      = (StrategyQueryFunc)     grid_query,
    .debug_draw = (StrategyDebugDrawFunc) grid_debug_draw,
};

static const Strategy STRATEGY_SPATIAL_HASHING = {
    .new        = (StrategyNewFunc)       spatial_hash_new,
    .free       = (StrategyFreeFunc)      spatial_hash_free,
    .insert     = (StrategyInsertFunc)    spatial_hash_insert,
    .clear      = (StrategyClearFunc)     spatial_hash_clear,
    .query      = (StrategyQueryFunc)     spatial_hash_query,
    .debug_draw = (StrategyDebugDrawFunc) spatial_hash_debug_draw,
};

static const Strategy STRATEGY_NAIVE = {
    .new        = (StrategyNewFunc)       naive_new,
    .free       = (StrategyFreeFunc)      naive_free,
    .insert     = (StrategyInsertFunc)    naive_insert,
    .clear      = (StrategyClearFunc)     naive_clear,
    .query      = (StrategyQueryFunc)     naive_query,
    .debug_draw = (StrategyDebugDrawFunc) naive_debug_draw,
};

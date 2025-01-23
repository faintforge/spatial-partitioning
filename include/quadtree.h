#pragma once

#include "box.h"
#include "ds.h"

#include <stdint.h>

#include <SDL2/SDL.h>

#define MAX_BOX_COUNT 128

typedef struct QuadtreeNode QuadtreeNode;
struct QuadtreeNode {
    QuadtreeNode *nw; // North west - Top left
    QuadtreeNode *ne; // North east - Top right
    QuadtreeNode *sw; // South west - Bottom left
    QuadtreeNode *se; // South east - Bottom right

    Box boxes[MAX_BOX_COUNT];
    size_t box_i;
    bool devided;

    Box area;
};

typedef struct Quadtree Quadtree;
struct Quadtree {
    QuadtreeNode *node_pool;
    size_t node_pool_i;

    uint32_t max_depth;
    uint32_t max_box_count;
};

typedef struct QuadtreeDesc QuadtreeDesc;
struct QuadtreeDesc {
    Box area;
    int max_depth;
    int max_box_count;
};

extern Quadtree* quadtree_new(const QuadtreeDesc* desc);
extern void quadtree_free(Quadtree *quadtree);

extern void quadtree_insert(Quadtree *quadtree, Box box);
extern void quadtree_clear(Quadtree *quadtree);

extern Vec(Box) quadtree_query(const Quadtree* quadtree, Box area);

extern void quadtree_debug_draw(const Quadtree* quadtree, SDL_Renderer *renderer);

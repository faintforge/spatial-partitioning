#pragma once

#include <box.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "ds.h"

#define GRID_MAX_BOX_COUNT 512

typedef struct Cell Cell;
struct Cell {
    Box boxes[GRID_MAX_BOX_COUNT];
    uint32_t box_i;
};

typedef struct Grid Grid;
struct Grid {
    Box world_box;
    Vec2 cell_count;
    Cell *cells;
};

extern Grid grid_new(Box grid_size, Vec2 cell_count);
extern void grid_free(Grid *grid);
extern void grid_insert(Grid *grid, Box box);
extern Vec(Box) grid_query(Grid grid, Box area);
extern void grid_reset(Grid *grid);
extern void grid_debug_draw(Grid grid, SDL_Renderer *renderer);

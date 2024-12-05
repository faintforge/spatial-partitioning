#include "grid.h"
#include "ds.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

Grid grid_new(Box grid_size, Vec2 cell_count) {
    Grid grid = {
        .world_box = grid_size,
        .cell_count = cell_count,
        .cells = malloc(cell_count.x*cell_count.y*sizeof(Cell)),
    };
    return grid;
}

void grid_free(Grid *grid) {
    free(grid->cells);
}

void grid_insert(Grid *grid, Box box) {
    const Vec2 cell_size = vec2_div(grid->world_box.size, grid->cell_count);

    Vec2 top_left = vec2_div(box.pos, cell_size);
    top_left.x = floorf(top_left.x);
    top_left.y = floorf(top_left.y);

    Vec2 bottom_right = vec2_div(vec2_add(box.pos, box.size), cell_size);
    bottom_right.x = ceilf(bottom_right.x);
    bottom_right.y = ceilf(bottom_right.y);

    for (int32_t y = top_left.y; y < bottom_right.y; y++) {
        for (int32_t x = top_left.x; x < bottom_right.x; x++) {
            Cell *cell = &grid->cells[x+y*(int) grid->cell_count.x];
            cell->boxes[cell->box_i++] = box;
            if (cell->box_i >= GRID_MAX_BOX_COUNT) {
                printf("Exceeding max cell capacity.");
            }
        }
    }
}

Vec(Box) grid_query(Grid grid, Box area) {
    const Vec2 cell_size = vec2_div(grid.world_box.size, grid.cell_count);

    Vec2 top_left = vec2_div(area.pos, cell_size);
    top_left.x = floorf(top_left.x);
    top_left.y = floorf(top_left.y);

    Vec2 bottom_right = vec2_div(vec2_add(area.pos, area.size), cell_size);
    bottom_right.x = ceilf(bottom_right.x);
    bottom_right.y = ceilf(bottom_right.y);

    Vec(Box) result = NULL;
    for (int32_t y = top_left.y; y < bottom_right.y; y++) {
        for (int32_t x = top_left.x; x < bottom_right.x; x++) {
            Cell cell = grid.cells[x+y*(int) grid.cell_count.x];
            for (uint32_t i = 0; i < cell.box_i; i++) {
                vec_push(result, cell.boxes[i]);
            }
        }
    }

    return result;
}

void grid_reset(Grid *grid) {
    for (uint32_t i = 0; i < grid->cell_count.x*grid->cell_count.y; i++) {
        grid->cells[i].box_i = 0;
    }
}

void grid_debug_draw(Grid grid, SDL_Renderer *renderer) {
    const Vec2 cell_size = vec2_div(grid.world_box.size, grid.cell_count);
    Vec2 pos = vec2s(0.0f); 
    for (uint32_t y = 0; y < grid.cell_count.y; y++) {
        for (uint32_t x = 0; x < grid.cell_count.x; x++) {
            Cell cell = grid.cells[x+y*(int) grid.cell_count.x];
            SDL_Rect rect = {
                .x = pos.x,
                .y = pos.y,
                .w = cell_size.x,
                .h = cell_size.y,
            };
            SDL_RenderDrawRect(renderer, &rect);
            pos.x += cell_size.x;
        }
        pos.x = 0.0f;
        pos.y += cell_size.y;
    }
}

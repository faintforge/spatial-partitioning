// NOTE: When changing the 'max_box_count' parameter on the quadtree, from 2 to
// 8, the performance increased by 10 ms.

#include "benchmark.h"
#include "color.h"
#include "hashing.h"
#include "window.h"
#include "ds.h"
#include "box.h"
#include "quadtree.h"
#include "grid.h"
#include "strategy_interface.h"

#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>

static float frand(void) {
    return (float) rand() / RAND_MAX;
}

typedef struct Config Config;
struct Config {
    uint32_t box_size;
    struct {
        uint32_t count;
        uint32_t init_box_count;
        uint32_t max_box_count;
    } iter;
    struct {
        uint32_t width;
        uint32_t height;
    } world;
};

const Config config = {
    .box_size = 4,
    .iter = {
        .count = 32,
        .init_box_count = 0,
        .max_box_count = 1000,
    },
    .world = {
        .width = 1280,
        .height = 720,
    }
};
#define BOX_INCREASE += 10

static void init_boxes(Vec(Box) *boxes, uint32_t count) {
    srand(1234);
    for (size_t i = 0; i < count; i++) {
        Box box = {
            .pos = vec2(frand()*(config.world.width-config.box_size), frand()*(config.world.height-config.box_size)),
            .size = vec2s(config.box_size),
        };
        vec_push(*boxes, box);
    }
}

static void benchmark(Window window, Strategy strat, const void* desc, const char* name) {
    for (uint32_t box_count = config.iter.init_box_count; box_count <= config.iter.max_box_count; box_count BOX_INCREASE) {
        printf("%s: Benchmarking %u boxes with %u iterations...\n", name, box_count, config.iter.count);

        Vec(Box) boxes = NULL;
        init_boxes(&boxes, box_count);

        void* data = strat.new(desc);

        // Insert all the boxes into the space.
        for (size_t i = 0; i < config.iter.count; i++) {
            for (size_t i = 0; i < vec_len(boxes); i++) {
                strat.insert(data, boxes[i]);
            }

            // Check for collisions.
            Vec(Box) colliding_boxes = NULL;
            Vec(Box) non_colliding_boxes = NULL;
            for (size_t i = 0; i < vec_len(boxes); i++) {
                bool collided = false;
                Vec(Box) near = strat.query(data, boxes[i]);
                for (size_t j = 0; j < vec_len(near); j++) {
                    if (box_eq(boxes[i], near[j])) {
                        continue;
                    }

                    if (box_overlapp(boxes[i], near[j])) {
                        vec_push(colliding_boxes, boxes[i]);
                        collided = true;
                        break;
                    }
                }
                vec_free(near);
                if (!collided) {
                    vec_push(non_colliding_boxes, boxes[i]);
                }
            }

            // Visualize.
            window_clear(window, color_rgb_hex(0x000000));
            SDL_SetRenderDrawColor(window.renderer, 64, 64, 64, 255);
            strat.debug_draw(data, window.renderer);

            SDL_SetRenderDrawColor(window.renderer, 255, 0, 0, 255);
            for (size_t i = 0; i < vec_len(colliding_boxes); i++) {
                SDL_FRect rect = {
                    .x = colliding_boxes[i].pos.x,
                    .y = colliding_boxes[i].pos.y,
                    .w = colliding_boxes[i].size.w,
                    .h = colliding_boxes[i].size.h,
                };
                SDL_RenderDrawRectF(window.renderer, &rect);
            }

            SDL_SetRenderDrawColor(window.renderer, 255, 255, 255, 255);
            for (size_t i = 0; i < vec_len(non_colliding_boxes); i++) {
                SDL_FRect rect = {
                    .x = non_colliding_boxes[i].pos.x,
                    .y = non_colliding_boxes[i].pos.y,
                    .w = non_colliding_boxes[i].size.w,
                    .h = non_colliding_boxes[i].size.h,
                };
                SDL_RenderDrawRectF(window.renderer, &rect);
            }

            window_present(window);

            strat.clear(data);
            vec_free(colliding_boxes);
            vec_free(non_colliding_boxes);
        }

        strat.free(data);
        vec_free(boxes);
    }
}

int32_t main(void) {
    Window window = window_create("Spatial Partitioning", config.world.width, config.world.height);

    const Box world_box = {
        .pos = {{0.0f, 0.0f}},
        .size = {{config.world.width, config.world.height}},
    };

    benchmark(window, STRATEGY_NAIVE, NULL, "Naive");

    QuadtreeDesc qt_desc = {
        .area = world_box,
        .max_depth = 8,
        .max_box_count = 8,
    };
    benchmark(window, STRATEGY_QUADTREE, &qt_desc, "Quadtree");

    GridDesc grid_desc = {
        .grid_size = world_box,
        .cell_count = vec2(16, 16),
    };
    benchmark(window, STRATEGY_GRID, &grid_desc, "Grid");

    SpatialHashDesc sh_desc = {
        .cell_size = vec2s(100.0f),
        .map_capacity = 4096,
    };
    benchmark(window, STRATEGY_SPATIAL_HASHING, &sh_desc, "Spatial Hashing");

    window_destroy(&window);
    return 0;
}

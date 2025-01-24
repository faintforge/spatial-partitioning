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

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>

static bool run_visually = true;

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
    .box_size = 2,
    .iter = {
        .count = 32,
        .init_box_count = 10,
        .max_box_count = 2500,
    },
    .world = {
        .width = 1280,
        .height = 720,
    }
};
#define BOX_INCREASE += 10

typedef void (*RandomPointsFunc)(Vec2* points, int count, Vec2 space);

void even_distribution(Vec2* points, int count, Vec2 space) {
    for (int i = 0; i < count; i++) {
        points[i] = vec2_mul(vec2(frand(), frand()), space);
    }
}

// Probability density function
float pdf(Vec2 point) {
    Vec2 c = vec2s(0.5f);
    Vec2 delta = vec2_sub(point, c);
    // Gaussian falloff
    return expf(-(delta.x * delta.x + delta.y * delta.y) / 0.1f);
}

void uneven_distribution(Vec2* points, int count, Vec2 space) {
    int i = 0;
    while (i < count) {
        Vec2 point = vec2(frand(), frand());
        float p = pdf(point);
        if (frand() < p) {
            points[i++] = vec2_mul(point, space);
        }
    }
}

static void init_boxes(Vec(Box) *boxes, uint32_t count, RandomPointsFunc rand_points_func) {
    srand(1234);

    Vec2* pos = malloc(sizeof(Vec2) * count);
    rand_points_func(pos, count, vec2(config.world.width - config.box_size, config.world.height - config.box_size));
    for (size_t i = 0; i < count; i++) {
        Box box = {
            .pos = pos[i],
            .size = vec2s(config.box_size),
        };
        vec_push(*boxes, box);
    }
    free(pos);
}

static void run(Window* window, Strategy strat, const void* desc, const char* name, RandomPointsFunc rand_points_func) {
    for (uint32_t box_count = config.iter.init_box_count; box_count <= config.iter.max_box_count; box_count BOX_INCREASE) {
        printf("%s: Benchmarking %u boxes with %u iterations...\n", name, box_count, config.iter.count);

        Vec(Box) boxes = NULL;
        init_boxes(&boxes, box_count, rand_points_func);

        void* data = strat.new(desc);

        bm_begin("%u", box_count);
        for (size_t i = 0; i < config.iter.count; i++) {
            // Insert all the boxes into the space.
            bm_begin("insert");
            for (size_t i = 0; i < vec_len(boxes); i++) {
                strat.insert(data, boxes[i]);
            }
            bm_end();

            // Check for collisions.
            bm_begin("collision");
            Vec(Box) colliding_boxes = NULL;
            Vec(Box) non_colliding_boxes = NULL;
            for (size_t i = 0; i < vec_len(boxes); i++) {
                // Query
                bm_begin("query");
                Vec(Box) near = strat.query(data, boxes[i]);
                bm_end();

                bool collided = false;
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
            bm_end();

            // Visualize.
            if (run_visually) {
                window_clear(*window, color_rgb_hex(0x000000));
                SDL_SetRenderDrawColor(window->renderer, 64, 64, 64, 255);
                strat.debug_draw(data, window->renderer);

                SDL_SetRenderDrawColor(window->renderer, 255, 0, 0, 255);
                for (size_t i = 0; i < vec_len(colliding_boxes); i++) {
                    SDL_FRect rect = {
                        .x = colliding_boxes[i].pos.x,
                        .y = colliding_boxes[i].pos.y,
                        .w = colliding_boxes[i].size.w,
                        .h = colliding_boxes[i].size.h,
                    };
                    SDL_RenderDrawRectF(window->renderer, &rect);
                }

                SDL_SetRenderDrawColor(window->renderer, 255, 255, 255, 255);
                for (size_t i = 0; i < vec_len(non_colliding_boxes); i++) {
                    SDL_FRect rect = {
                        .x = non_colliding_boxes[i].pos.x,
                        .y = non_colliding_boxes[i].pos.y,
                        .w = non_colliding_boxes[i].size.w,
                        .h = non_colliding_boxes[i].size.h,
                    };
                    SDL_RenderDrawRectF(window->renderer, &rect);
                }

                window_present(*window);
                window_poll_events(window);

                if (!window->is_open) {
                    exit(0);
                }
            }

            bm_begin("clear");
            strat.clear(data);
            bm_end();

            vec_free(colliding_boxes);
            vec_free(non_colliding_boxes);
        }
        bm_end();

        strat.free(data);
        vec_free(boxes);
    }
}

int32_t main(void) {
    run_visually = false;

    Window* window = NULL;
    if (run_visually) {
        window = malloc(sizeof(Window));
        *window = window_create("Spatial Partitioning", config.world.width, config.world.height);
    }

    const Box world_box = {
        .pos = {{0.0f, 0.0f}},
        .size = {{config.world.width, config.world.height}},
    };

    {
        // Naive
        // bm_begin("naive");
        // run(window, STRATEGY_NAIVE, NULL, "Naive", even_distribution);
        // bm_end();

        // Gird
        GridDesc grid_desc = {
            .grid_size = world_box,
            .cell_count = vec2(16, 16),
        };
        bm_begin("Grid");
        run(window, STRATEGY_GRID, &grid_desc, "Grid", even_distribution);
        bm_end();

        // Quadtree
        QuadtreeDesc qt_desc = {
            .area = world_box,
            .max_depth = 8,
            .max_box_count = 8,
        };
        bm_begin("Quadtree");
        run(window, STRATEGY_QUADTREE, &qt_desc, "Quadtree", even_distribution);
        bm_end();

        // Spatial hashing
        SpatialHashDesc sh_desc = {
            .cell_size = vec2s(100.0f),
            .map_capacity = 4096,
        };
        bm_begin("Spatial Hashing");
        run(window, STRATEGY_SPATIAL_HASHING, &sh_desc, "Spatial Hashing", even_distribution);
        bm_end();

        // bm_dump();
        bm_dump_json("benchmark-even.json");
    }

    bm_reset();

    {
        // Naive
        // bm_begin("naive");
        // run(window, STRATEGY_NAIVE, NULL, "Naive", uneven_distribution);
        // bm_end();

        // Gird
        GridDesc grid_desc = {
            .grid_size = world_box,
            .cell_count = vec2(16, 16),
        };
        bm_begin("Grid");
        run(window, STRATEGY_GRID, &grid_desc, "Grid", uneven_distribution);
        bm_end();

        // Quadtree
        QuadtreeDesc qt_desc = {
            .area = world_box,
            .max_depth = 8,
            .max_box_count = 8,
        };
        bm_begin("Quadtree");
        run(window, STRATEGY_QUADTREE, &qt_desc, "Quadtree", uneven_distribution);
        bm_end();

        // Spatial hashing
        SpatialHashDesc sh_desc = {
            .cell_size = vec2s(100.0f),
            .map_capacity = 4096,
        };
        bm_begin("Spatial Hashing");
        run(window, STRATEGY_SPATIAL_HASHING, &sh_desc, "Spatial Hashing", uneven_distribution);
        bm_end();

        bm_dump_json("benchmark-uneven.json");
    }

    if (run_visually) {
        window_destroy(window);
    }

    return 0;
}

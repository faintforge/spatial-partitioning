// NOTE: When changing the 'max_box_count' parameter on the quadtree, from 2 to
// 8, the performance increased by 10 ms.

#include "benchmark.h"
#include "color.h"
#include "hashing.h"
#include "window.h"

#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <time.h>
#include <unistd.h>

#include <SDL2/SDL_timer.h>
#include <stdint.h>
#include <stdlib.h>

#include "ds.h"
#include "box.h"
#include "quadtree.h"
#include "grid.h"

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

static void benchmark_naive(Window window) {
    Benchmark *bm = NULL;

    for (uint32_t box_count = config.iter.init_box_count; box_count <= config.iter.max_box_count; box_count BOX_INCREASE) {
        printf("Naive: Benchmarking %u boxes with %u iterations...\n", box_count, config.iter.count);

        Vec(Box) boxes = NULL;
        init_boxes(&boxes, box_count);

        Vec(double) iter_times = NULL;
        for (size_t i = 0; i < config.iter.count; i++) {
            window_clear(window, color_rgb_hex(0x000000));

            Vec(Box) colliding_boxes = NULL;
            Vec(Box) non_colliding_boxes = NULL;

            bench_func(iter_time) {
                for (uint32_t i = 0; i < box_count; i++) {
                    bool collided = false;
                    for (uint32_t j = 0; j < box_count; j++) {
                        if (i == j) {
                            continue;
                        }

                        if (box_overlapp(boxes[i], boxes[j])) {
                            vec_push(colliding_boxes, boxes[i]);
                            collided = true;
                            break;
                        }
                    }
                    if (!collided) {
                        vec_push(non_colliding_boxes, boxes[i]);
                    }
                }
            }

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

            vec_push(iter_times, iter_time);

            vec_free(colliding_boxes);
            vec_free(non_colliding_boxes);
        }

        benchmark_register(&bm, iter_times, box_count);
        vec_free(boxes);
    }

    benchmark_write_json(bm, "naive.json");
    benchmark_free(bm);
}

static void benchmark_grid(Window window) {
    Benchmark *bm = NULL;

    for (uint32_t box_count = config.iter.init_box_count; box_count <= config.iter.max_box_count; box_count BOX_INCREASE) {
        printf("Grid: Benchmarking %u boxes with %u iterations...\n", box_count, config.iter.count);
        // Initialize
        Vec(Box) boxes = NULL;
        init_boxes(&boxes, box_count);

        const Box world_box = {
            .pos = {{0.0f, 0.0f}},
            .size = {{config.world.width, config.world.height}},
        };
        Grid grid = grid_new(world_box, vec2(16, 16));

        Vec(double) iter_times = NULL;
        // Collision testing
        for (size_t i = 0; i < config.iter.count; i++) {
            Vec(Box) colliding_boxes = NULL;
            Vec(Box) non_colliding_boxes = NULL;

            window_clear(window, color_rgb_hex(0x000000));
            bench_func(iter_time) {
                for (size_t i = 0; i < vec_len(boxes); i++) {
                    grid_insert(&grid, boxes[i]);
                }
                for (size_t i = 0; i < vec_len(boxes); i++) {
                    bool collided = false;
                    Vec(Box) near = grid_query(grid, boxes[i]);
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
            }

            SDL_SetRenderDrawColor(window.renderer, 64, 64, 64, 255);
            grid_debug_draw(grid, window.renderer);

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

            bench_func(clear_time) {
                grid_reset(&grid);
            }

            vec_push(iter_times, clear_time + iter_time);

            vec_free(colliding_boxes);
            vec_free(non_colliding_boxes);
        }

        benchmark_register(&bm, iter_times, box_count);

        grid_free(&grid);
        vec_free(boxes);
    }

    benchmark_write_json(bm, "quadtree.json");
    benchmark_free(bm);
}

static void benchmark_spatial_hashing(Window *window) {
    Vec(Box) boxes = NULL;
    init_boxes(&boxes, 128);

    uint32_t cell_width = 100;
    uint32_t cell_height = 100;
    SpatialHash space = spatial_hash_new(4096, vec2(cell_width, cell_height));

    while (window->is_open) {
        for (size_t i = 0; i < vec_len(boxes); i++) {
            spatial_hash_insert(&space, boxes[i]);
        }

        window_clear(*window, color_rgb_hex(0x000000));

        SDL_SetRenderDrawColor(window->renderer, 64, 64, 64, 255);
        spatial_hash_debug_draw(space, config.world.width / cell_width + 1, config.world.height / cell_height + 1, window->renderer);

        SDL_SetRenderDrawColor(window->renderer, 255, 255, 255, 255);
        for (size_t i = 0; i < vec_len(boxes); i++) {
            SDL_FRect rect = {
                .x = boxes[i].pos.x,
                .y = boxes[i].pos.y,
                .w = boxes[i].size.w,
                .h = boxes[i].size.h,
            };
            SDL_RenderDrawRectF(window->renderer, &rect);
        }

        int32_t x, y;
        SDL_GetMouseState(&x, &y);
        Vec(Box) query = spatial_hash_query(space, (Box) {
                .pos = vec2(x, y),
                .size = vec2s(1.0f),
            });

        SDL_SetRenderDrawColor(window->renderer, 255, 0, 0, 255);
        for (size_t i = 0; i < vec_len(query); i++) {
            SDL_FRect rect = {
                .x = query[i].pos.x,
                .y = query[i].pos.y,
                .w = query[i].size.w,
                .h = query[i].size.h,
            };
            SDL_RenderDrawRectF(window->renderer, &rect);
        }
        vec_free(query);

        window_present(*window);
        window_poll_events(window);

        spatial_hash_clear(&space);
    }

    spatial_hash_free(&space);
}

static void benchmark_quadtree(Window window) {
    Benchmark *bm = NULL;

    for (uint32_t box_count = config.iter.init_box_count; box_count <= config.iter.max_box_count; box_count BOX_INCREASE) {
        printf("Quadtree: Benchmarking %u boxes with %u iterations...\n", box_count, config.iter.count);
        // Initialize
        Vec(Box) boxes = NULL;
        init_boxes(&boxes, box_count);

        const Box world_box = {
            .pos = {{0.0f, 0.0f}},
            .size = {{config.world.width, config.world.height}},
        };
        Quadtree quadtree = quadtree_new(world_box, 8, 8);

        Vec(double) iter_times = NULL;
        // Collision testing
        for (size_t i = 0; i < config.iter.count; i++) {
            Vec(Box) colliding_boxes = NULL;
            Vec(Box) non_colliding_boxes = NULL;

            window_clear(window, color_rgb_hex(0x000000));
            bench_func(iter_time) {
                for (size_t i = 0; i < vec_len(boxes); i++) {
                    quadtree_insert(&quadtree, boxes[i]);
                }
                for (size_t i = 0; i < vec_len(boxes); i++) {
                    bool collided = false;
                    Vec(Box) near = quadtree_query(quadtree, boxes[i]);
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
            }

            SDL_SetRenderDrawColor(window.renderer, 64, 64, 64, 255);
            quadtree_debug_draw(quadtree, window.renderer);

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

            bench_func(clear_time) {
                quadtree_clear(&quadtree);
            }

            vec_push(iter_times, clear_time + iter_time);

            vec_free(colliding_boxes);
            vec_free(non_colliding_boxes);
        }

        benchmark_register(&bm, iter_times, box_count);

        quadtree_free(&quadtree);
        vec_free(boxes);
    }

    benchmark_write_json(bm, "grid.json");
    benchmark_free(bm);
}

int32_t main(void) {
    Window window = window_create("Spatial Partitioning", config.world.width, config.world.height);

    // benchmark_naive(window);
    // benchmark_grid(window);
    benchmark_spatial_hashing(&window);
    // benchmark_quadtree(window);

    window_destroy(&window);
    return 0;
}

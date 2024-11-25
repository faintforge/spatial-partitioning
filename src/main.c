// NOTE: When changing the 'max_box_count' parameter on the quadtree, from 2 to
// 8, the performance increased by 10 ms.

#include "benchmark.h"
#include "color.h"
#include "window.h"

#include <SDL2/SDL_render.h>
#include <time.h>
#include <unistd.h>

#include <SDL2/SDL_timer.h>
#include <stdint.h>
#include <stdlib.h>

#include "ds.h"
#include "box.h"
#include "quadtree.h"

static float frand(void) {
    return (float) rand() / RAND_MAX;
}

const uint32_t BOX_SIZE = 2;
const uint32_t WORLD_WIDTH = 1280; 
const uint32_t WORLD_HEIGHT = 720; 

static void init_boxes(Vec(Box) *boxes, uint32_t count) {
    srand(1234);
    for (size_t i = 0; i < count; i++) {
        Box box = {
            .pos = vec2(frand()*(WORLD_WIDTH-BOX_SIZE), frand()*(WORLD_HEIGHT-BOX_SIZE)),
            .size = vec2s(BOX_SIZE),
        };
        vec_push(*boxes, box);
    }
}

static void benchmark_quadtree(Window *window) {
    uint32_t iter_count = 32;

    Benchmark *bm = NULL;

    for (uint32_t box_count = 1; box_count <= 1<<14; box_count *= 2) {
        printf("Quadtree: Benchmarking %u boxes with %u iterations...\n", box_count, iter_count);
        // Initialize
        Vec(Box) boxes = NULL;
        init_boxes(&boxes, box_count);

        const Box world_box = {
            .pos = {{0.0f, 0.0f}},
            .size = {{WORLD_WIDTH, WORLD_HEIGHT}},
        };
        Quadtree quadtree = quadtree_new(world_box, 8, 8);

        Vec(Box) colliding_boxes = NULL;
        Vec(Box) non_colliding_boxes = NULL;

        Vec(double) iter_times = NULL;
        // Collision testing
        for (size_t i = 0; i < iter_count; i++) {
            window_clear(*window, color_rgb_hex(0x000000));
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

            SDL_SetRenderDrawColor(window->renderer, 64, 64, 64, 255);
            quadtree_debug_draw(quadtree, window->renderer);

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

            bench_func(clear_time) {
                quadtree_clear(&quadtree);
            }

            vec_push(iter_times, clear_time + iter_time);
        }

        benchmark_register(&bm, iter_times, box_count);

        quadtree_free(&quadtree);
        vec_free(boxes);
    }

    benchmark_write_json(bm, "quadtree.json");
    benchmark_free(bm);
}

static void benchmark_naive(void) {
}

static void benchmark_hashmap(void) {
}

int32_t main(void) {
    Window window = window_create("Spatial Partitioning", WORLD_WIDTH, WORLD_HEIGHT);

    benchmark_quadtree(&window);

    window_destroy(&window);
    return 0;
}

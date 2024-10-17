#include "window.h"

#include <SDL2/SDL_timer.h>
#include <stdint.h>
#include <stdlib.h>

#include "ds.h"
#include "box.h"
#include "quadtree.h"

static float frand(void) {
    return (float) rand() / RAND_MAX;
}

int32_t main(void) {
    const uint32_t BOX_SIZE = 4;
    const uint32_t BOX_COUNT = 1024;

    Window window = window_create("Spatial Partitioning", 800, 600);

    srand(1234);

    Vec(Box) boxes = NULL;
    for (size_t i = 0; i < BOX_COUNT; i++) {
        Box box = {
            .pos = vec2(frand()*(window.width-BOX_SIZE), frand()*(window.height-BOX_SIZE)),
            .size = vec2s(BOX_SIZE),
        };
        vec_push(boxes, box);
    }

    Quadtree quadtree = quadtree_new(box(0, 0, window.width, window.height), 8, 2);

    uint64_t last = SDL_GetTicks64();
    float dt = 0.0;
    uint32_t fps = 0;
    float fps_timer = 0.0;
    while (window.is_open) {
        uint64_t curr = SDL_GetTicks64();
        dt = (curr - last) * 0.001;
        last = curr;

        // fps++;
        // fps_timer += dt;
        // if (fps_timer >= 1.0) {
        //     printf("FPS: %u\n", fps);
        //     fps_timer = 0.0;
        //     fps = 0;
        // }

        window_clear(window, color_rgb_hex(0x000000));

        Vec(Box) colliding_boxes = NULL;
        Vec(Box) non_colliding_boxes = NULL;

        // -- Quadtree Collision detection -------------------------------------
        uint64_t start_time = SDL_GetTicks64();
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

        // SDL_SetRenderDrawColor(window.renderer, 128, 128, 128, 255);
        // quadtree_debug_draw(quadtree, window.renderer);

        quadtree_clear(&quadtree);
        uint64_t end_time = SDL_GetTicks64();
        uint64_t collision_time_delta = end_time - start_time;
        printf("Collision time: %lu ms\n", collision_time_delta);

        // -- Naive Collision detection ----------------------------------------
        // uint64_t start_time = SDL_GetTicks64();
        // for (size_t i = 0; i < vec_len(boxes); i++) {
        //     bool collided = false;
        //     for (size_t j = 0; j < vec_len(boxes); j++) {
        //         if (i == j) {
        //             continue;
        //         }
        //
        //         if (box_overlapp(boxes[i], boxes[j])) {
        //             vec_push(colliding_boxes, boxes[i]);
        //             collided = true;
        //             break;
        //         }
        //     }
        //
        //     if (!collided) {
        //         vec_push(non_colliding_boxes, boxes[i]);
        //     }
        // }
        // uint64_t end_time = SDL_GetTicks64();
        // uint64_t collision_time_delta = end_time - start_time;
        // printf("Collision time: %lu ms\n", collision_time_delta);

        SDL_SetRenderDrawColor(window.renderer, 255, 255, 255, 255);
        for (size_t i = 0; i < vec_len(non_colliding_boxes); i++) {
            SDL_Rect rect = {
                .x = non_colliding_boxes[i].pos.x,
                .y = non_colliding_boxes[i].pos.y,
                .w = non_colliding_boxes[i].size.x,
                .h = non_colliding_boxes[i].size.y,
            };
            SDL_RenderDrawRect(window.renderer, &rect);
        }

        SDL_SetRenderDrawColor(window.renderer, 255, 0, 0, 255);
        for (size_t i = 0; i < vec_len(colliding_boxes); i++) {
            SDL_Rect rect = {
                .x = colliding_boxes[i].pos.x,
                .y = colliding_boxes[i].pos.y,
                .w = colliding_boxes[i].size.x,
                .h = colliding_boxes[i].size.y,
            };
            SDL_RenderDrawRect(window.renderer, &rect);
        }

        window_present(window);

        window_poll_events(&window);
    }
    vec_free(boxes);

    quadtree_free(&quadtree);

    window_destroy(&window);

    return 0;
}

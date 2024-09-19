#include "window.h"

#include <SDL2/SDL_timer.h>
#include <stdint.h>
#include <stdlib.h>

#include "ds.h"
#include "box.h"

static float frand(void) {
    return (float) rand() / RAND_MAX;
}

int32_t main(void) {
    const uint32_t BOX_SIZE = 50;

    Window window = window_create("Spatial Partitioning", 800, 600);

    srand(1234);

    Vec(Box) boxes = NULL;
    for (size_t i = 0; i < 8; i++) {
        Box box = {
            .pos = vec2(frand()*(window.width-BOX_SIZE), frand()*(window.height-BOX_SIZE)),
            .size = vec2s(BOX_SIZE),
        };
        vec_push(boxes, box);
    }

    uint64_t last = SDL_GetTicks64();
    float dt = 0.0;
    uint32_t fps = 0;
    float fps_timer = 0.0;
    while (window.is_open) {
        uint64_t curr = SDL_GetTicks64();
        dt = (curr - last) * 0.001;
        last = curr;

        fps++;
        fps_timer += dt;
        if (fps_timer >= 1.0) {
            printf("FPS: %u\n", fps);
            fps_timer = 0.0;
            fps = 0;
        }

        window_clear(window, color_rgb_hex(0x000000));

        SDL_SetRenderDrawColor(window.renderer, 255, 255, 255, 255);
        for (size_t i = 0; i < vec_len(boxes); i++) {
            SDL_Rect rect = {
                .x = boxes[i].pos.x,
                .y = boxes[i].pos.y,
                .w = boxes[i].size.x,
                .h = boxes[i].size.y,
            };
            SDL_RenderDrawRect(window.renderer, &rect);
        }
        window_present(window);

        window_poll_events(&window);
    }
    vec_free(boxes);

    window_destroy(&window);

    return 0;
}

#include "window.h"

#include <SDL2/SDL_timer.h>
#include <stdint.h>

int32_t main(void) {
    Window window = window_create("Spatial Partitioning", 800, 600);

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

        SDL_SetRenderDrawColor(window.renderer, 0, 0, 0, 255);
        SDL_RenderClear(window.renderer);

        SDL_Rect rect = {
            .x = 50,
            .y = 50,
            .w = 50,
            .h = 50,
        };
        SDL_SetRenderDrawColor(window.renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(window.renderer, &rect);
        SDL_RenderPresent(window.renderer);

        window_poll_events(&window);
    }
    window_destroy(&window);

    return 0;
}

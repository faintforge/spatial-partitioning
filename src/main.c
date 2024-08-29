#include "arkin_core.h"
#include "arkin_log.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

I32 main(void) {
    arkin_init(&(ArkinCoreDesc) {
            .error.callback = ar_log_error_callback,
        });
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        ar_error("SDL::Init");
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Spatial Partitioning", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        ar_error("SDL::WindowCreation");
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (window == NULL) {
        ar_error("SDL::RendererCreation");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    B8 running = true;
    while (running) {
        SDL_Event ev = {0};
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
                case SDL_QUIT:
                    running = false;
                    break;
            }
        }

        SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
        SDL_RenderClear(rend);

        SDL_Rect rect = {
            .x = 50,
            .y = 50,
            .w = 50,
            .h = 50,
        };
        SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
        SDL_RenderDrawRect(rend, &rect);
        SDL_RenderPresent(rend);
    }
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);

    SDL_Quit();
    arkin_terminate();
    return 0;
}

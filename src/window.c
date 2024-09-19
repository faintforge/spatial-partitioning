#include "window.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

Window window_create(const char *title, uint32_t width, uint32_t height) {
    Window window = {0};

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "ERR:SDL::Init\n");
        return window;
    }

    SDL_Window *handle = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (handle == NULL) {
        fprintf(stderr, "ERR::SDL::WindowCreation\n");
        SDL_Quit();
        return window;
    }
    window.handle = handle;

    SDL_Renderer *rend = SDL_CreateRenderer(handle, -1, SDL_RENDERER_ACCELERATED);
    if (handle == NULL) {
        fprintf(stderr, "ERR::SDL::RendererCreation\n");
        SDL_DestroyWindow(handle);
        SDL_Quit();
        return window;
    }
    window.renderer = rend;
    window.is_open = true;

    window.width = width;
    window.height = height;

    return window;
}

void window_destroy(Window *window) {
    SDL_DestroyRenderer(window->renderer);
    SDL_DestroyWindow(window->handle);
    SDL_Quit();
    *window = (Window) {0};
}

void window_poll_events(Window *window) {
    SDL_Event ev = {0};
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_QUIT:
                window->is_open = false;
                break;
        }
    }
}

void window_clear(Window window, Color color) {
    SDL_SetRenderDrawColor(window.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(window.renderer);
}

void window_present(Window window) {
    SDL_RenderPresent(window.renderer);
}

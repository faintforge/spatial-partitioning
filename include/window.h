#pragma once

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Window Window;
struct Window {
    SDL_Window *handle;
    SDL_Renderer *renderer;
    bool is_open;
};

extern Window window_create(const char *title, uint32_t width, uint32_t height);
extern void window_destroy(Window *window);

extern void window_poll_events(Window *window);

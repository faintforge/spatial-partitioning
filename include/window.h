#pragma once

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include "arkin_core.h"

typedef struct Window Window;
struct Window {
    SDL_Window *handle;
    SDL_Renderer *renderer;
    B8 is_open;
};

extern Window window_create(const char *title, U32 width, U32 height);
extern void window_destroy(Window *window);

extern void window_poll_events(Window *window);

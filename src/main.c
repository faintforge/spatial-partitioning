#include "arkin_core.h"
#include "arkin_log.h"

#include "window.h"

I32 main(void) {
    arkin_init(&(ArkinCoreDesc) {
            .error.callback = ar_log_error_callback,
        });

    Window window = window_create("Spatial Partitioning", 800, 600);

    while (window.is_open) {
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

    arkin_terminate();
    return 0;
}

#include "naive.h"
#include "ds.h"

Naive* naive_new(const void* desc) {
    (void) desc;
    Naive* naive = malloc(sizeof(Naive));
    *naive = (Naive) {0};
    return naive;
}

void naive_free(Naive* data) {
    vec_free(data->boxes);
}

void naive_insert(Naive* data, Box box) {
    vec_push(data->boxes, box);
}

void naive_clear(Naive* data) {
    vec_free(data->boxes);
}

Vec(Box) naive_query(const Naive* data, Box area) {
    (void) area;
    Vec(Box) result = NULL;
    vec_insert_arr(result, 0, data->boxes, vec_len(data->boxes));
    return result;
}

void naive_debug_draw(const Naive* data, SDL_Renderer* renderer) {
    (void) data;
    (void) renderer;
}

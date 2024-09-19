#pragma once

#include <stdint.h>

typedef struct Color Color;
struct Color {
    uint8_t r, g, b, a;
};

static inline Color color_rgb(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { return (Color) {r, g, b, a}; }
static inline Color color_rgba(uint8_t r, uint8_t g, uint8_t b) { return (Color) {r, g, b, 255 }; }

static inline Color color_rgb_hex(uint32_t hex) {
    return (Color) {
        .r = hex >> (8 * 2) & 0xff,
        .g = hex >> (8 * 1) & 0xff,
        .b = hex >> (8 * 0) & 0xff,
        .a = 255,
    };
}
static inline Color color_rgba_hex(uint32_t hex) {
    return (Color) {
        .r = hex >> (8 * 3) & 0xff,
        .g = hex >> (8 * 2) & 0xff,
        .b = hex >> (8 * 1) & 0xff,
        .a = hex >> (8 * 0) & 0xff,
    };
}

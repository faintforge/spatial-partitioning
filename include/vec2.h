#pragma once

typedef union Vec2 Vec2;
union Vec2 {
    struct { float x, y; };
    struct { float w, h; };
    float f[2];
};

static inline Vec2 vec2(float x, float y) { return (Vec2) {{x, y}}; }
static inline Vec2 vec2s(float scaler) { return (Vec2) {{scaler, scaler}}; }

// Element wise vector multiplication.
static inline Vec2 vec2_mul(Vec2 a, Vec2 b) { return (Vec2) {{a.x*b.x, a.y*b.y}}; }
// Element wise vector division.
static inline Vec2 vec2_div(Vec2 a, Vec2 b) { return (Vec2) {{a.x/b.x, a.y/b.y}}; }
// Element wise vector addition.
static inline Vec2 vec2_add(Vec2 a, Vec2 b) { return (Vec2) {{a.x+b.x, a.y+b.y}}; }
// Element wise vector subtraction.
static inline Vec2 vec2_sub(Vec2 a, Vec2 b) { return (Vec2) {{a.x-b.x, a.y-b.y}}; }

// Scale vector.
static inline Vec2 vec2_muls(Vec2 a, float scaler) { return (Vec2) {{a.x*scaler, a.y*scaler}}; }
// Shrink vector.
static inline Vec2 vec2_divs(Vec2 a, float scaler) { return (Vec2) {{a.x/scaler, a.y/scaler}}; }
// Add scaler to all elements of vector.
static inline Vec2 vec2_adds(Vec2 a, float scaler) { return (Vec2) {{a.x+scaler, a.y+scaler}}; }
// Subtract scaler from all elements of vector.
static inline Vec2 vec2_subs(Vec2 a, float scaler) { return (Vec2) {{a.x-scaler, a.y-scaler}}; }

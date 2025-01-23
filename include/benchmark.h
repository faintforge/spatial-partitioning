#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "ds.h"

typedef struct Benchmark Benchmark;
struct Benchmark {
    // Stack of benchmarks.
    Benchmark *next;

    const char* name;
    uint32_t run_count;
    double total_ms;
    double average_ms;
    double min_ms;
    double max_ms;
    HashMap(const char*, Benchmark*) children;
};

extern double get_time(void);

extern void bm_begin(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
extern void bm_end(void);
#define bm(fmt, ...) for (bool _i_ = (bm_begin(name, ##__VA_ARGS__), false); !_i_; _i_ = (bm_end(), true))

extern void bm_dump(void);

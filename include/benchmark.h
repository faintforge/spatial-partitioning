#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "ds.h"

typedef struct Benchmark Benchmark;
struct Benchmark {
    // Stack of benchmarks.
    Benchmark *next;
    uint32_t box_count;
    Vec(double) iter_times;

    double total_ms;
    double average_ms;
    double min_ms;
    double max_ms;
};

extern double get_time(void);

#define bench_func(t) \
    double t = 0.0; \
    for (bool _i_ = (t = get_time(), false); !_i_; _i_ = (t = get_time() - t, true))

// Pushes a benchmark onto the stack.
// Don't free 'iter_times' it's stored within the benchmark. It is freed when
// 'benchmark_free' is called.
extern void benchmark_register(Benchmark **benchmark, Vec(double) iter_times, uint32_t box_count);
extern void benchmark_free(Benchmark *benchmark);
extern void benchmark_write_json(Benchmark *benchmark, const char *filename);

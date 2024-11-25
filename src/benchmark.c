#include "benchmark.h"
#include "ds.h"

#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

// Get the unix time stamp in miliseconds.
double get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    static struct timespec init_ts = {0};
    if (init_ts.tv_sec == 0) {
        init_ts = ts;
    }

    return (double) (ts.tv_sec-init_ts.tv_sec) * 1e3 + (double) (ts.tv_nsec-init_ts.tv_nsec) / 1e6;
}

void benchmark_free(Benchmark *benchmark) {
    if (benchmark == NULL) {
        return;
    }

    benchmark_free(benchmark->next);
    vec_free(benchmark->iter_times);
    free(benchmark);
}

void benchmark_register(Benchmark **benchmark, Vec(double) iter_times, uint32_t box_count) {
    double total = 0.0f;
    double min = INFINITY;
    double max = 0.0f;
    for (size_t i = 0; i < vec_len(iter_times); i++) {
        total += iter_times[i];
        if (iter_times[i] < min) {
            min = iter_times[i];
        }
        if (iter_times[i] > max) {
            max = iter_times[i];
        }
    }

    Benchmark *bm = malloc(sizeof(Benchmark));
    *bm = (Benchmark) {
        .next = *benchmark,
        .box_count = box_count,
        .iter_times = iter_times,

        .total_ms = total,
        .min_ms = min,
        .max_ms = max,
        .average_ms = total / vec_len(iter_times),
    };
    *benchmark = bm;
}

void benchmark_write_json(Benchmark *benchmark, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        fprintf(stderr, "IO: Failed to open file '%s'.\n", filename);
        exit(1);
    }

    fprintf(fp, "[");
    for (Benchmark *bm = benchmark; bm != NULL; bm = bm->next) {
        fprintf(fp, "{");

        fprintf(fp, "\"box_count\": %u,", bm->box_count);
        fprintf(fp, "\"total\": %f,", bm->total_ms);
        fprintf(fp, "\"average\": %f,", bm->average_ms);
        fprintf(fp, "\"min\": %f,", bm->min_ms);
        fprintf(fp, "\"max\": %f", bm->max_ms);

        // fprintf(fp, "\"times\": [");
        // for (size_t i = 0; i < vec_len(bm->iter_times); i++) {
        //     fprintf(fp, "%f", bm->iter_times[i]);
        //     if (i < vec_len(bm->iter_times)-1) {
        //         fprintf(fp, ",");
        //     }
        // }
        // fprintf(fp, "]");

        fprintf(fp, "}");
        if (bm->next != NULL) {
            fprintf(fp, ",");
        }
    }
    fprintf(fp, "]");

    fclose(fp);
}

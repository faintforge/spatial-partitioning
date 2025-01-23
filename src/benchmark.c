#include "benchmark.h"
#include "ds.h"

#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <float.h>

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

static size_t str_hash(const void* data, size_t len) {
    (void) len;
    char* const* str = data;
    return fvn1a_hash(*str, strlen(*str));
}

static int str_cmp(const void* a, const void* b, size_t len) {
    (void) len;
    char* const* _a = a;
    char* const* _b = b;
    return strcmp(*_a, *_b);
}

Benchmark root = {0};
Benchmark *curr = NULL;

void bm_begin(const char* fmt, ...) {
    HashMapDesc desc = hash_map_desc_default(root.children);
    desc.hash = str_hash;
    desc.cmp = str_cmp;

    va_list args;
    va_start(args, fmt);
    va_list args_len;
    va_copy(args_len, args);
    int len = vsnprintf(NULL, 0, fmt, args_len);
    va_end(args_len);
    char* name = malloc(len + 1);
    vsnprintf(name, len + 1, fmt, args);
    va_end(args);

    if (root.children == NULL) {
        hash_map_new(root.children, desc);
    }

    Benchmark* bm;
    if (curr == NULL) {
        bm = hash_map_get(root.children, name);
    } else {
        bm = hash_map_get(curr->children, name);
    }

    // New benchmark
    if (bm == NULL) {
        bm = malloc(sizeof(Benchmark));
        *bm = (Benchmark) {
            .name = name,
            .min_ms = DBL_MAX,
        };
        hash_map_new(bm->children, desc);
        if (curr == NULL) {
            hash_map_insert(root.children, name, bm);
        } else {
            hash_map_insert(curr->children, name, bm);
        }
    } else {
        free(name);
    }

    bm->run_count++;
    bm->_start = get_time();

    bm->next = curr;
    curr = bm;
}

void bm_end(void) {
    double time = get_time() - curr->_start;
    curr->total_ms += time;
    curr->average_ms = curr->total_ms / curr->run_count;
    if (curr->min_ms > time) {
        curr->min_ms = time;
    }
    if (curr->max_ms < time) {
        curr->max_ms = time;
    }

    curr = curr->next;
}

void bm_dump_helper(const Benchmark* bm, int depth) {
    if (bm == NULL) {
        return;
    }

    char spaces[512] = {0};
    memset(spaces, ' ', 4 * depth);
    printf("%s%s: (runs: %d), (total: %.2f ms)\n",
        spaces,
        bm->name,
        bm->run_count,
        bm->average_ms);

    HashMapIter iter = hash_map_iter_new(bm->children);
    while (hash_map_iter_valid(bm->children, iter)) {
        bm_dump_helper(bm->children[iter].value, depth + 1);
        iter = hash_map_iter_next(bm->children, iter);
    }
}

void bm_dump(void) {
    printf("---------- Benchmark Dump ----------\n");
    HashMapIter iter = hash_map_iter_new(root.children);
    while (hash_map_iter_valid(root.children, iter)) {
        bm_dump_helper(root.children[iter].value, 0);
        iter = hash_map_iter_next(root.children, iter);
    }
}

void bm_dump_json_helper(FILE* fp, const Benchmark* bm, int depth) {
    if (bm == NULL) {
        return;
    }

    char spaces[512] = {0};

    // Object opening
    memset(spaces, ' ', 4 * depth);
    fprintf(fp, "%s\"%s\": {\n", spaces, bm->name);

    // Run count
    memset(spaces, ' ', 4 * (depth + 1));
    fprintf(fp, "%s\"run_count\": %u,\n", spaces, bm->run_count);

    // Average time
    fprintf(fp, "%s\"average\": %f,\n", spaces, bm->average_ms);

    // Total time
    fprintf(fp, "%s\"total\": %f,\n", spaces, bm->total_ms);

    // Min time
    fprintf(fp, "%s\"min\": %f,\n", spaces, bm->min_ms);

    // Max time
    fprintf(fp, "%s\"max\": %f,\n", spaces, bm->max_ms);

    // Children
    fprintf(fp, "%s\"children\": {\n", spaces);

    HashMapIter iter = hash_map_iter_new(bm->children);
    while (hash_map_iter_valid(bm->children, iter)) {
        bm_dump_json_helper(fp, bm->children[iter].value, depth + 2);
        HashMapIter iter_next = hash_map_iter_next(bm->children, iter);
        if (hash_map_iter_valid(bm->children, iter_next)) {
            fprintf(fp, ",\n");
        } else {
            fprintf(fp, "\n");
        }
        iter = iter_next;
    }
    fprintf(fp, "%s}\n", spaces);

    // Object close
    memset(spaces, ' ', 4 * depth);
    spaces[4 * depth] = '\0';
    fprintf(fp, "%s}", spaces);
}

void bm_dump_json(const char* filename) {
    FILE* fp = fopen(filename, "wb");
    fprintf(fp, "{\n");
    HashMapIter iter = hash_map_iter_new(root.children);
    while (hash_map_iter_valid(root.children, iter)) {
        bm_dump_json_helper(fp, root.children[iter].value, 1);
        HashMapIter iter_next = hash_map_iter_next(root.children, iter);
        if (hash_map_iter_valid(root.children, iter_next)) {
            fprintf(fp, ",\n");
        } else {
            fprintf(fp, "\n");
        }
        iter = iter_next;
    }
    fprintf(fp, "}");
    fclose(fp);
}

void bm_reset(void) {
    hash_map_free(root.children);
    curr = NULL;
}

#include "ds.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

//
// Vector
//

#define VEC_INITIAL_CAPACITY 8

typedef struct VecHeader VecHeader;
struct VecHeader {
    size_t element_size;
    size_t capacity;
    size_t len;
};

#define vec_to_header(vec) ((VecHeader*) ((uint8_t*) (vec) - sizeof(VecHeader)))
#define header_to_vec(header) ((void*) ((uint8_t*) (header) + sizeof(VecHeader)))

size_t vec_len(const void* vec) {
    if (vec == NULL) {
        return 0;
    }

    return vec_to_header(vec)->len;
}

size_t vec_capacity(const void* vec) {
    if (vec == NULL) {
        return 0;
    }

    return vec_to_header(vec)->capacity;
}

size_t vec_element_size(const void *vec) {
    if (vec == NULL) {
        return 0;
    }

    return vec_to_header(vec)->element_size;
}

void *vec_new(size_t element_size) {
    VecHeader *header = malloc(sizeof(VecHeader) + element_size*VEC_INITIAL_CAPACITY);
    *header = (VecHeader) {
        .element_size = element_size,
        .capacity = VEC_INITIAL_CAPACITY,
    };

    return header_to_vec(header);
}

void _vec_free(void** vec) {
    if (*vec == NULL) {
        return;
    }

    free(vec_to_header(*vec));

    *vec = NULL;
}

static void vec_ensure_capacity(void** vec, size_t len) {
    VecHeader *header = vec_to_header(*vec);
    if (header->capacity >= header->len + len) {
        return;
    }

    while (header->len + len > header->capacity) {
        header->capacity *= 2;
    }

    header = realloc(header, sizeof(VecHeader) + header->element_size*header->capacity);
    if (header == NULL) {
        *vec = NULL;
        return;
    }
    *vec = header_to_vec(header);
}

void _vec_insert_arr(void** vec, size_t index, const void* arr, size_t len) {
    VecHeader *header = vec_to_header(*vec);
    assert(index <= header->len);
    vec_ensure_capacity(vec, len);
    header = vec_to_header(*vec);

    void *end = (*vec) + (index+len)*header->element_size;
    void *dest = (*vec) + index*header->element_size;

    memmove(end, dest, (header->len - index)*header->element_size);
    if (arr == NULL) {
        memset(dest, 0, len*header->element_size);
    } else {
        memcpy(dest, arr, len*header->element_size);
    }


    header->len += len;
}

void _vec_remove_arr(void** vec, size_t index, size_t len, void *result) {
    VecHeader *header = vec_to_header(*vec);
    assert(index+len <= header->len);

    void *end = (*vec) + (index+len)*header->element_size;
    void *dest = (*vec) + index*header->element_size;

    if (result != NULL) {
        memcpy(result, dest, len*header->element_size);
    }
    memmove(dest, end, (header->len - index - len)*header->element_size);

    header->len -= len;
}

void _vec_insert_fast(void** vec, size_t index, const void* element) {
    VecHeader *header = vec_to_header(*vec);
    assert(index <= header->len);
    vec_ensure_capacity(vec, 1);
    header = vec_to_header(*vec);

    void *end = (*vec) + header->len*header->element_size;
    void *dest = (*vec) + index*header->element_size;

    memcpy(end, dest, header->element_size);
    if (element == NULL) {
        memset(dest, 0, header->element_size);
    } else {
        memcpy(dest, element, header->element_size);
    }

    header->len += 1;
}

void _vec_remove_fast(void** vec, size_t index, void *result) {
    VecHeader *header = vec_to_header(*vec);
    assert(index+1 <= header->len);

    void *end = (*vec) + (header->len-1)*header->element_size;
    void *dest = (*vec) + index*header->element_size;

    if (result != NULL) {
        memcpy(result, dest, header->element_size);
    }
    memcpy(dest, end, header->element_size);

    header->len -= 1;
}

//
// HashSet
//

#define SET_INITIAL_CAPACITY 8
#define SET_FILL_LIMIT 0.75

typedef enum {
    HASH_SET_SLOT_EMPTY,
    HASH_SET_SLOT_ALIVE,
    HASH_SET_SLOT_DEAD,
} HashSetSlotStatus;

typedef struct HashSetHeader HashSetHeader;
struct HashSetHeader {
    HashSetDesc desc;
    size_t capacity;
    size_t count;
    HashSetSlotStatus *statuses;
};

#define hash_set_to_header(set) ((HashSetHeader *) ((void *) (set) - sizeof(HashSetHeader)))
#define header_to_hash_set(header) ((void *) (header) + sizeof(HashSetHeader))

void *hash_set_new(HashSetDesc desc) {
    HashSetHeader *header = malloc(sizeof(HashSetHeader) + SET_INITIAL_CAPACITY*desc.element_size);
    *header = (HashSetHeader) {
        .desc = desc,
        .capacity = SET_INITIAL_CAPACITY,
        .statuses = calloc(SET_INITIAL_CAPACITY, sizeof(HashSetSlotStatus)),
    };

    return header_to_hash_set(header);
}

void _hash_set_free(void **set) {
    HashSetHeader *header = hash_set_to_header(*set);

    free(header->statuses);
    free(header);
    *set = NULL;
}

size_t hash_set_capacity(const void *set) {
    if (set == NULL) {
        return 0;
    }
    return hash_set_to_header(set)->capacity;
}

size_t hash_set_count(const void *set) {
    if (set == NULL) {
        return 0;
    }
    return hash_set_to_header(set)->count;
}

static void hash_set_resize(HashSetHeader **header, size_t new_capacity) {
    HashSetSlotStatus *new_statuses = calloc(new_capacity, sizeof(HashSetSlotStatus));
    HashSetHeader *new_header = malloc(sizeof(HashSetHeader) + (*header)->desc.element_size*new_capacity);
    void *new_elements = header_to_hash_set(new_header);
    void *old_elements = header_to_hash_set(*header);

    *new_header = **header;
    new_header->capacity = new_capacity;
    new_header->statuses = new_statuses;

    for (size_t i = 0; i < (*header)->capacity; i++) {
        if ((*header)->statuses[i] == HASH_SET_SLOT_ALIVE) {
            const void *old_element = old_elements + i*(*header)->desc.element_size;
            size_t index = (*header)->desc.hash(old_element, (*header)->desc.element_size) % new_capacity;

            while (new_statuses[index] == HASH_SET_SLOT_ALIVE) {
                index = (index + 1) % new_capacity;
            }

            new_statuses[index] = HASH_SET_SLOT_ALIVE;
            memcpy(new_elements + index*(*header)->desc.element_size, old_element, (*header)->desc.element_size);
        }
    }

    free((*header)->statuses);
    free(*header);
    *header = new_header;
}

void _hash_set_insert(void **set, const void *element) {
    HashSetHeader *header = hash_set_to_header(*set);

    size_t index = header->desc.hash(element, header->desc.element_size) % header->capacity;

    void *curr_element;
    while (true) {
        HashSetSlotStatus status = header->statuses[index];
        curr_element = *set + header->desc.element_size*index;

        if (status == HASH_SET_SLOT_ALIVE) {
            // Nested if statments because otherwise the else would catch this condition too
            // meaning I'd have to specify every other status in an 'else if' block.
            if (header->desc.cmp(curr_element, element, header->desc.element_size) == 0) {
                return;
            }
        } else {
            break;
        }

        index = (index + 1) % header->capacity;
    }

    header->statuses[index] = HASH_SET_SLOT_ALIVE;
    memcpy(curr_element, element, header->desc.element_size);
    header->count++;

    if (header->count >= header->capacity*SET_FILL_LIMIT) {
        hash_set_resize(&header, header->capacity*2);
        *set = header_to_hash_set(header);
    }
}

void _hash_set_remove(void **set, const void *element) {
    HashSetHeader *header = hash_set_to_header(*set);
    size_t index = header->desc.hash(element, header->desc.element_size) % header->capacity;

    while (true) {
        HashSetSlotStatus status = header->statuses[index];
        const void *curr_element = *set + header->desc.element_size*index;

        if ((status == HASH_SET_SLOT_ALIVE &&
            header->desc.cmp(curr_element, element, header->desc.element_size) == 0) ||
            status == HASH_SET_SLOT_EMPTY) {
            break;
        }

        index = (index + 1) % header->capacity;
    }

    header->statuses[index] = HASH_SET_SLOT_DEAD;
    header->count--;

    // Shrink the hash set to make iteration faster since there's less things to loop over.
    if (header->count <= header->capacity/4) {
        hash_set_resize(&header, header->capacity/2);
        *set = header_to_hash_set(header);
    }
}

bool _hash_set_contains(const void *set, const void *element) {
    const HashSetHeader *header = hash_set_to_header(set);
    size_t index = header->desc.hash(element, header->desc.element_size) % header->capacity;

    while (true) {
        HashSetSlotStatus status = header->statuses[index];
        const void *curr_element = set + header->desc.element_size*index;

        if (status == HASH_SET_SLOT_EMPTY) {
            break;
        } else if (status == HASH_SET_SLOT_ALIVE) {
            if (header->desc.cmp(curr_element, element, header->desc.element_size) == 0) {
                return true;
            }
        }

        index = (index + 1) % header->capacity;
    }

    return false;
}

void *hash_set_union(const void *a, const void *b) {
    assert(a != NULL);
    assert(b != NULL);

    const HashSetHeader *a_header = hash_set_to_header(a);
    const HashSetHeader *b_header = hash_set_to_header(b);
    assert(a_header->desc.element_size == b_header->desc.element_size);

    void *result = hash_set_new(a_header->desc);

    for (size_t i = 0; i < a_header->capacity; i++) {
        if (a_header->statuses[i] != HASH_SET_SLOT_ALIVE) {
            continue;
        }

        const void *element = a + i*a_header->desc.element_size;
        _hash_set_insert(&result, element);
    }

    for (size_t i = 0; i < b_header->capacity; i++) {
        if (b_header->statuses[i] != HASH_SET_SLOT_ALIVE) {
            continue;
        }

        const void *element = b + i*b_header->desc.element_size;
        _hash_set_insert(&result, element);
    }

    return result;
}

void *hash_set_intersect(const void *a, const void *b) {
    assert(a != NULL);
    assert(b != NULL);

    const HashSetHeader *a_header = hash_set_to_header(a);
    const HashSetHeader *b_header = hash_set_to_header(b);
    assert(a_header->desc.element_size == b_header->desc.element_size);

    HashSetHeader *result = hash_set_new(a_header->desc);

    for (size_t i = 0; i < a_header->capacity; i++) {
        if (a_header->statuses[i] != HASH_SET_SLOT_ALIVE) {
            continue;
        }

        const int *element = a + i*a_header->desc.element_size;
        if (_hash_set_contains(b, element)) {
            _hash_set_insert((void **) &result, element);
        }
    }

    return result;
}

void *hash_set_difference(const void *a, const void *b) {
    assert(a != NULL);
    assert(b != NULL);

    const HashSetHeader *a_header = hash_set_to_header(a);
    const HashSetHeader *b_header = hash_set_to_header(b);
    assert(a_header->desc.element_size == b_header->desc.element_size);

    HashSetHeader *result = hash_set_new(a_header->desc);

    for (size_t i = 0; i < a_header->capacity; i++) {
        if (a_header->statuses[i] != HASH_SET_SLOT_ALIVE) {
            continue;
        }

        const void *element = a + i*a_header->desc.element_size;
        if (!_hash_set_contains(b, element)) {
            _hash_set_insert((void **) &result, element);
        }
    }

    return result;
}

// Iteration
HashSetIter hash_set_iter_new(const void *set) {
    if (set == NULL) {
        return -1;
    }

    const HashSetHeader *header = hash_set_to_header(set);
    for (size_t i = 0; i < header->capacity; i++) {
        if (header->statuses[i] == HASH_SET_SLOT_ALIVE) {
            return i;
        }
    }

    return header->capacity;
}

bool hash_set_iter_valid(const void *set, HashSetIter iter) {
    if (set == NULL) {
        return false;
    }

    const HashSetHeader *header = hash_set_to_header(set);
    return iter < header->capacity;
}

HashSetIter hash_set_iter_next(const void *set, HashSetIter iter) {
    if (set == NULL) {
        return -1;
    }

    const HashSetHeader *header = hash_set_to_header(set);
    for (size_t i = iter+1; i < header->capacity; i++) {
        if (header->statuses[i] == HASH_SET_SLOT_ALIVE) {
            return i;
        }
    }

    return header->capacity;
}

//
// HashMap
//

typedef enum {
    HASH_MAP_BUCKET_EMPTY,
    HASH_MAP_BUCKET_ALIVE,
    HASH_MAP_BUCKET_DEAD,
} HashMapBucketState;

typedef struct HashMapHeader HashMapHeader;
struct HashMapHeader {
    HashMapDesc desc;
    HashMapBucketState *states;
    size_t *hashes;
    size_t capacity;
    size_t count;
    void *zero_value;
};

#define header_to_hash_map(header) ((void *) header + sizeof(HashMapHeader))
#define hash_map_to_header(map) ((HashMapHeader *) (map - sizeof(HashMapHeader)))
#define HASH_MAP_INITIAL_CAPACITY 8
#define HASH_MAP_FILL_LIMIT 0.75f

void _hash_map_new(void **map, HashMapDesc desc) {
    if (*map != NULL) {
        return;
    }

    size_t bucket_size = desc.bucket_size;
    HashMapHeader *header = malloc(sizeof(HashMapHeader) + HASH_MAP_INITIAL_CAPACITY*bucket_size);
    *header = (HashMapHeader) {
        .desc = desc,
        .states = calloc(HASH_MAP_INITIAL_CAPACITY, sizeof(HashMapBucketState)),
        .hashes = calloc(HASH_MAP_INITIAL_CAPACITY, sizeof(size_t)),
        .capacity = HASH_MAP_INITIAL_CAPACITY,
        .zero_value = malloc(desc.value_size),
    };
    if (desc.zero_value != NULL) {
        memcpy(header->zero_value, desc.zero_value, desc.value_size);
    } else {
        memset(header->zero_value, 0, desc.value_size);
    }

    *map = header_to_hash_map(header);
}

void _hash_map_free(void **map) {
    if (*map == NULL) {
        return;
    }

    HashMapHeader *header = hash_map_to_header(*map);
    free(header->states);
    free(header->hashes);
    free(header->zero_value);
    free(header);
    *map = NULL;
}

// Returns the index of an empty bucket or an alive bucket matching the key.
static size_t hash_map_get_bucket(HashMapHeader *header, const void *key, size_t hash) {
    size_t index = hash % header->capacity;

    void *map = header_to_hash_map(header);
    size_t bucket_size = header->desc.bucket_size;

    while (true) {
        HashMapBucketState state = header->states[index];
        void *bucket_key = map + index*bucket_size;

        if ((state == HASH_MAP_BUCKET_ALIVE &&
            header->hashes[index] == hash &&
            header->desc.cmp(bucket_key, key, header->desc.key_size) == 0) ||
            state == HASH_MAP_BUCKET_EMPTY) {
            break;
        }

        index = (index + 1) % header->capacity;
    }

    return index;
}

static void hash_map_resize(HashMapHeader **header, size_t new_capacity) {
    size_t bucket_size = (*header)->desc.bucket_size;
    HashMapHeader *new_header = malloc(sizeof(HashMapHeader) + new_capacity*bucket_size);
    *new_header = **header;
    new_header->capacity = new_capacity;
    new_header->states = calloc(new_capacity, sizeof(HashMapBucketState));
    new_header->hashes = calloc(new_capacity, sizeof(size_t));

    for (size_t i = 0; i < (*header)->capacity; i++) {
        if ((*header)->states[i] != HASH_MAP_BUCKET_ALIVE) {
            continue;
        }

        const void *old_key = header_to_hash_map(*header) + bucket_size*i;
        size_t index = hash_map_get_bucket(new_header, old_key, (*header)->hashes[i]);

        void *new_key = header_to_hash_map(new_header) + bucket_size*index;
        memcpy(new_key, old_key, (*header)->desc.key_size);

        const void *old_value = header_to_hash_map(*header) + bucket_size*i + (*header)->desc.value_offset;
        void *new_value = header_to_hash_map(new_header) + bucket_size*index + (*header)->desc.value_offset;
        memcpy(new_value, old_value, (*header)->desc.value_size);

        new_header->hashes[index] = (*header)->hashes[i];
        new_header->states[index] = HASH_MAP_BUCKET_ALIVE;
    }

    free((*header)->states);
    free((*header)->hashes);
    free(*header);
    *header = new_header;
}

void _hash_map_insert(void **map, const void *key, const void *value) {
    if (*map == NULL) {
        return;
    }

    HashMapHeader *header = hash_map_to_header(*map);

    size_t hash = header->desc.hash(key, header->desc.key_size);
    size_t index = hash_map_get_bucket(header, key, hash);
    HashMapBucketState *state = &header->states[index];
    if (*state != HASH_MAP_BUCKET_EMPTY) {
        return;
    }

    size_t bucket_size = header->desc.bucket_size;
    void *bucket_key = *map + index*bucket_size;
    void *bucket_value = bucket_key+header->desc.value_offset;

    memcpy(bucket_key, key, header->desc.key_size);
    memcpy(bucket_value, value, header->desc.value_size);
    *state = HASH_MAP_BUCKET_ALIVE;
    header->hashes[index] = hash;

    header->count++;

    if (header->count >= header->capacity*HASH_MAP_FILL_LIMIT) {
        hash_map_resize(&header, header->capacity*2);
        *map = header_to_hash_map(header);
    }
}

void _hash_map_set(void **map, const void *key, const void *value) {
    if (*map == NULL) {
        return;
    }

    HashMapHeader *header = hash_map_to_header(*map);

    size_t hash = header->desc.hash(key, header->desc.key_size);
    size_t index = hash_map_get_bucket(header, key, hash);
    HashMapBucketState *state = &header->states[index];

    size_t bucket_size = header->desc.bucket_size;
    void *bucket_key = *map + index*bucket_size;
    void *bucket_value = bucket_key+header->desc.value_offset;

    if (*state == HASH_MAP_BUCKET_EMPTY) {
        header->count++;
    }

    memcpy(bucket_key, key, header->desc.key_size);
    memcpy(bucket_value, value, header->desc.value_size);
    *state = HASH_MAP_BUCKET_ALIVE;
    header->hashes[index] = hash;
}

HashMapIter _hash_map_remove(void **map, const void *key) {
    if (*map == NULL) {
        return -1;
    }

    HashMapHeader *header = hash_map_to_header(*map);

    // Always resize before deleting because the index will be returned and if
    // the map is resized after remove operation has finished the index will be
    // invalid.
    if (header->count <= header->capacity/4) {
        hash_map_resize(&header, header->capacity/2);
        *map = header_to_hash_map(header);
    }

    size_t hash = header->desc.hash(key, header->desc.key_size);
    size_t index = hash_map_get_bucket(header, key, hash);
    HashMapBucketState *state = &header->states[index];
    if (*state != HASH_MAP_BUCKET_ALIVE) {
        return header->capacity;
    }

    *state = HASH_MAP_BUCKET_DEAD;
    header->count--;

    return index;
}

void _hash_map_get(const void *map, const void *key, void *result) {
    HashMapHeader *header = hash_map_to_header(map);

    size_t hash = header->desc.hash(key, header->desc.key_size);
    size_t index = hash_map_get_bucket(header, key, hash);
    HashMapBucketState state = header->states[index];
    if (state != HASH_MAP_BUCKET_ALIVE) {
        memcpy(result, header->zero_value, header->desc.value_size);
        return;
    }

    size_t bucket_size = header->desc.bucket_size;
    const void *bucket_value = map + index*bucket_size+header->desc.value_offset;
    memcpy(result, bucket_value, header->desc.value_size);
}

void *_hash_map_getp(void *map, const void *key) {
    HashMapHeader *header = hash_map_to_header(map);

    size_t hash = header->desc.hash(key, header->desc.key_size);
    size_t index = hash_map_get_bucket(header, key, hash);
    HashMapBucketState state = header->states[index];
    if (state != HASH_MAP_BUCKET_ALIVE) {
        return NULL;
    }

    size_t bucket_size = header->desc.bucket_size;
    void *bucket_value = map + index*bucket_size+header->desc.value_offset;
    return bucket_value;
}

size_t hash_map_count(const void *map) {
    return hash_map_to_header(map)->count;
}

// Iteration
HashMapIter hash_map_iter_new(const void *map) {
    if (map == NULL) {
        return -1;
    }

    HashMapHeader *header = hash_map_to_header(map);
    for (size_t i = 0; i < header->capacity; i++) {
        if (header->states[i] == HASH_MAP_BUCKET_ALIVE) {
            return i;
        }
    }

    return header->capacity;
}

bool hash_map_iter_valid(const void *map, HashMapIter iter) {
    if (map == NULL) {
        return false;
    }

    HashMapHeader *header = hash_map_to_header(map);
    return iter < header->capacity;
}

HashMapIter hash_map_iter_next(const void *map, HashMapIter iter) {
    if (map == NULL) {
        return -1;
    }

    HashMapHeader *header = hash_map_to_header(map);
    for (size_t i = iter+1; i < header->capacity; i++) {
        if (header->states[i] == HASH_MAP_BUCKET_ALIVE) {
            return i;
        }
    }

    return header->capacity;
}

//
// Utils
//

size_t fvn1a_hash(const void *data, size_t len) {
    const char *_data = data;
    size_t hash = 2166136261u;
    for (size_t i = 0; i < len; i++) {
        hash ^= *(_data + i);
        hash *= 16777619;
    }
    return hash;
}

void *hash_set_to_vec(const void *set) {
    const HashSetHeader *set_header = hash_set_to_header(set);
    void *vec = vec_new(set_header->desc.element_size);

    for (size_t i = 0; i < set_header->capacity; i++) {
        if (set_header->statuses[i] == HASH_SET_SLOT_ALIVE) {
            _vec_insert_fast(&vec, vec_len(vec), set + i*set_header->desc.element_size);
        }
    }

    return vec;
}

void _vec_to_hash_set(const void *vec, void **hash_set) {
    VecHeader *vec_header = vec_to_header(vec);
    for (size_t i = 0; i < vec_len(vec); i++) {
        _hash_set_insert(hash_set, vec + i*vec_header->element_size);
    }
}

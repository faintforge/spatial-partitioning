#include "ds.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

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

//
// Vector
//

#define VEC_INITIAL_CAPACITY 8

typedef struct VecHeader VecHeader;
struct VecHeader {
    size_t item_size;
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

void _vec_create(void** vec, size_t item_size) {
    if (*vec != NULL) {
        return;
    }

    VecHeader *header = malloc(sizeof(VecHeader) + item_size*VEC_INITIAL_CAPACITY);
    *header = (VecHeader) {
        .item_size = item_size,
        .capacity = VEC_INITIAL_CAPACITY,
    };

    *vec = header_to_vec(header);
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

    header = realloc(header, sizeof(VecHeader) + header->item_size*header->capacity);
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

    void *end = (*vec) + (index+len)*header->item_size;
    void *dest = (*vec) + index*header->item_size;

    memmove(end, dest, (header->len - index)*header->item_size);
    memcpy(dest, arr, len*header->item_size);

    header->len += len;
}

void _vec_remove_arr(void** vec, size_t index, size_t len, void *result) {
    VecHeader *header = vec_to_header(*vec);
    assert(index+len <= header->len);

    void *end = (*vec) + (index+len)*header->item_size;
    void *dest = (*vec) + index*header->item_size;

    if (result != NULL) {
        memcpy(result, dest, len*header->item_size);
    }
    memmove(dest, end, (header->len - index - len)*header->item_size);

    header->len -= len;
}

void _vec_insert_fast(void** vec, size_t index, const void* element) {
    VecHeader *header = vec_to_header(*vec);
    assert(index <= header->len);
    vec_ensure_capacity(vec, 1);
    header = vec_to_header(*vec);

    void *end = (*vec) + header->len*header->item_size;
    void *dest = (*vec) + index*header->item_size;

    memcpy(end, dest, header->item_size);
    memcpy(dest, element, header->item_size);

    header->len += 1;
}

void _vec_remove_fast(void** vec, size_t index, void *result) {
    VecHeader *header = vec_to_header(*vec);
    assert(index+1 <= header->len);

    void *end = (*vec) + (header->len-1)*header->item_size;
    void *dest = (*vec) + index*header->item_size;

    if (result != NULL) {
        memcpy(result, dest, header->item_size);
    }
    memcpy(dest, end, header->item_size);

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
    void *elements;
    HashSetSlotStatus *statuses;
};

void *_hash_set_new(HashSetDesc desc) {
    HashSetHeader *header = malloc(sizeof(HashSetHeader));
    *header = (HashSetHeader) {
        .desc = desc,
        .capacity = SET_INITIAL_CAPACITY,
        .elements = malloc(desc.element_size*SET_INITIAL_CAPACITY),
        .statuses = calloc(SET_INITIAL_CAPACITY, sizeof(HashSetSlotStatus)),
    };

    return header;
}

void _hash_set_free(void **set) {
    HashSetHeader *header = *set;

    free(header->statuses);
    free(header->elements);
    free(header);
    *set = NULL;
}

static void hash_set_resize(HashSetHeader *header) {
    size_t old_capacity = header->capacity;
    header->capacity *= 2;

    HashSetSlotStatus *new_statuses = malloc(sizeof(HashSetSlotStatus)*header->capacity);
    void *new_elements = malloc(header->desc.element_size*header->capacity);

    for (size_t i = 0; i < old_capacity; i++) {
        if (header->statuses[i] == HASH_SET_SLOT_ALIVE) {
            const void *old_element = header->elements + i*header->desc.element_size;
            size_t index = header->desc.hash(old_element, header->desc.element_size) % header->capacity;

            while (new_statuses[index] == HASH_SET_SLOT_ALIVE) {
                index = (index + 1) % header->capacity;
            }

            new_statuses[index] = HASH_SET_SLOT_ALIVE;
            memcpy(new_elements + index*header->desc.element_size, old_element, header->desc.element_size);
        }
    }

    free(header->statuses);
    free(header->elements);

    header->statuses = new_statuses;
    header->elements = new_elements;
}

void _hash_set_insert(void **set, const void *element) {
    HashSetHeader *header = *set;
    if (header->count > header->capacity*SET_FILL_LIMIT) {
        hash_set_resize(header);
    }

    size_t index = header->desc.hash(element, header->desc.element_size) % header->capacity;
    
    void *curr_element;
    while (true) {
        HashSetSlotStatus status = header->statuses[index];
        curr_element = header->elements + header->desc.element_size*index;

        if (status == HASH_SET_SLOT_ALIVE) {
            // Nested if statments because otherwise the else would catch this condition too
            // meaning I'd have to specify every other status in an 'else if' block.
            if (header->desc.cmp(curr_element, element, header->desc.element_size) == 0) {
                break;
            }
        } else {
            break;
        }

        index = (index + 1) % header->capacity;
    }

    header->statuses[index] = HASH_SET_SLOT_ALIVE;
    memcpy(curr_element, element, header->desc.element_size);
    header->count++;
}

void _hash_set_remove(void **set, const void *element) {
    HashSetHeader *header = *set;
    size_t index = header->desc.hash(element, header->desc.element_size) % header->capacity;

    while (true) {
        HashSetSlotStatus status = header->statuses[index];
        const void *curr_element = header->elements + header->desc.element_size*index;

        if ((status == HASH_SET_SLOT_ALIVE &&
            header->desc.cmp(curr_element, element, header->desc.element_size) == 0) ||
            status == HASH_SET_SLOT_EMPTY) {
            break;
        }

        index = (index + 1) % header->capacity;
    }

    header->statuses[index] = HASH_SET_SLOT_DEAD;
    header->count--;
}

bool _hash_set_contains(const void *set, const void *element) {
    const HashSetHeader *header = set;
    size_t index = header->desc.hash(element, header->desc.element_size) % header->capacity;

    while (true) {
        HashSetSlotStatus status = header->statuses[index];
        const void *curr_element = header->elements + header->desc.element_size*index;

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

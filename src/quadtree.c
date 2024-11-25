#include "quadtree.h"
#include "box.h"

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_stdinc.h>
#include <math.h>
#include <stdlib.h>

static QuadtreeNode *quadtree_get_node(Quadtree *quadtree, Box area) {
    QuadtreeNode *node = &quadtree->node_pool[quadtree->node_pool_i++];
    *node = (QuadtreeNode) {
        .area = area,
    };
    return node;
}

static void quadtree_node_insert(Quadtree *quadtree, QuadtreeNode *node, Box box, uint32_t depth) {
    if (!box_overlapp(box, node->area)) {
        return;
    }

    if (depth == quadtree->max_depth-1) {
        if (node->box_i >= MAX_BOX_COUNT) {
            printf("WARN: Max box count exceeded for a single quadrant.\n");
            exit(1);
        }
        node->boxes[node->box_i++] = box;
        return;
    }

    if (node->box_i == quadtree->max_box_count) {
        node->nw = quadtree_get_node(quadtree, (Box) {
                .pos = {
                    .x = node->area.pos.x,
                    .y = node->area.pos.y,
                },
                .size = vec2_divs(node->area.size, 2.0f),
            });

        node->ne = quadtree_get_node(quadtree, (Box) {
                .pos = {
                    .x = node->area.pos.x + node->area.size.x/2.0f,
                    .y = node->area.pos.y,
                },
                .size = vec2_divs(node->area.size, 2.0f),
            });

        node->sw = quadtree_get_node(quadtree, (Box) {
                .pos = {
                    .x = node->area.pos.x,
                    .y = node->area.pos.y + node->area.size.y/2.0f,
                },
                .size = vec2_divs(node->area.size, 2.0f),
            });

        node->se = quadtree_get_node(quadtree, (Box) {
                .pos = {
                    .x = node->area.pos.x + node->area.size.x/2.0f,
                    .y = node->area.pos.y + node->area.size.y/2.0f,
                },
                .size = vec2_divs(node->area.size, 2.0f),
            });

        for (size_t i = 0; i < node->box_i; i++) {
            quadtree_node_insert(quadtree, node->nw, node->boxes[i], depth+1);
            quadtree_node_insert(quadtree, node->ne, node->boxes[i], depth+1);
            quadtree_node_insert(quadtree, node->sw, node->boxes[i], depth+1);
            quadtree_node_insert(quadtree, node->se, node->boxes[i], depth+1);
        }
        node->box_i = 0;

        node->devided = true;
    }

    if (node->devided) {
        quadtree_node_insert(quadtree, node->nw, box, depth+1);
        quadtree_node_insert(quadtree, node->ne, box, depth+1);
        quadtree_node_insert(quadtree, node->sw, box, depth+1);
        quadtree_node_insert(quadtree, node->se, box, depth+1);
        return;
    }

    node->boxes[node->box_i++] = box;
}

Quadtree quadtree_new(Box area, int max_depth, int max_box_count) {
    size_t max_node_count = powf(4, max_depth);
    Quadtree quadtree = {
        .max_depth = max_depth,
        .max_box_count = max_box_count,
        .node_pool = malloc(max_node_count * sizeof(QuadtreeNode)),
        .node_pool_i = 1,
    };

    quadtree.node_pool[0] = (QuadtreeNode) {
        .area = area,
    };

    return quadtree;
}

// void quadtree_node_free(QuadtreeNode *node) {
//     if (node == NULL) {
//         return;
//     }
//
//     // Use depth-first approach to free the deepest node in the tree first.
//     quadtree_node_free(node->nw);
//     quadtree_node_free(node->ne);
//     quadtree_node_free(node->sw);
//     quadtree_node_free(node->se);
//
//     vec_free(node->boxes);
//     free(node);
// }

void quadtree_free(Quadtree *quadtree) {
    free(quadtree->node_pool);
}

void quadtree_insert(Quadtree *quadtree, Box box) {
    quadtree_node_insert(quadtree, &quadtree->node_pool[0], box, 0);
}

void quadtree_clear(Quadtree *quadtree) {
    quadtree->node_pool_i = 1;
    quadtree->node_pool[0] = (QuadtreeNode) {
        .area = quadtree->node_pool[0].area,
    };
}

void quadtree_query_helper(QuadtreeNode *node, Box area, Vec(Box) *result) {
    if (node == NULL || !box_overlapp(node->area, area)) {
        return;
    }

    quadtree_query_helper(node->nw, area, result);
    quadtree_query_helper(node->ne, area, result);
    quadtree_query_helper(node->sw, area, result);
    quadtree_query_helper(node->se, area, result);

    vec_insert_arr(*result, vec_len(*result), node->boxes, node->box_i);
}

Vec(Box) quadtree_query(Quadtree quadtree, Box area) {
    Vec(Box) result = NULL;
    quadtree_query_helper(&quadtree.node_pool[0], area, &result);
    return result;
}

void quadtree_debug_draw_helper(QuadtreeNode *node, SDL_Renderer *renderer) {
    if (node == NULL) {
        return;
    }

    quadtree_debug_draw_helper(node->nw, renderer);
    quadtree_debug_draw_helper(node->ne, renderer);
    quadtree_debug_draw_helper(node->sw, renderer);
    quadtree_debug_draw_helper(node->se, renderer);

    SDL_Rect rect = {
        .x = node->area.pos.x,
        .y = node->area.pos.y,
        .w = node->area.size.x,
        .h = node->area.size.y,
    };
    SDL_RenderDrawRect(renderer, &rect);
}

void quadtree_debug_draw(Quadtree quadtree, SDL_Renderer *renderer) {
    quadtree_debug_draw_helper(&quadtree.node_pool[0], renderer);
}

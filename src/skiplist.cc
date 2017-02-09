/**
 * Copyright (C) 2017-present Jung-Sang Ahn <jungsang.ahn@gmail.com>
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>

#include "skiplist.h"

//#define __SL_DEBUG
#ifdef __SL_DEBUG
    #include "skiplist_debug.h"
#else
    #define __SLD_RT_INS(e, n, t, c)
    #define __SLD_NC_INS(n, nn, t, c)
    #define __SLD_RT_RMV(e, n, t, c)
    #define __SLD_NC_RMV(n, nn, t, c)
    #define __SLD_BM(n)
    #define __SLD_ASSERT(cond)
#endif

#if defined(_STL_ATOMIC) && defined(__cplusplus)
    // C++ (STL) atomic operations
    #define MOR                         std::memory_order_relaxed
    #define ATM_LOAD(var, val)          (val) = (var).load(MOR)
    #define ATM_STORE(var, val)         (var).store((val), MOR)
    #define ATM_CAS(var, exp, val)      (var).compare_exchange_weak((exp), (val))
    #define ALLOC_NODE_PTR(var, count)  (var) = new atm_node_ptr[count]
    #define FREE_NODE_PTR(var)          delete[] (var)
#else
    // C-style atomic operations
    #ifndef __cplusplus
        typedef uint8_t bool;

        #ifndef true
            #define true 1
        #endif

        #ifndef false
            #define false 0
        #endif
    #endif

    #define MOR                         __ATOMIC_RELAXED
    #define ATM_LOAD(var, val)          __atomic_load(&(var), &(val), MOR)
    #define ATM_STORE(var, val)         __atomic_store(&(var), &(val), MOR)
    #define ATM_CAS(var, exp, val)      \
            __atomic_compare_exchange(&(var), &(exp), &(val), 1, MOR, MOR)
    #define ALLOC_NODE_PTR(var, count)  \
            (var) = (atm_node_ptr*)calloc(count, sizeof(atm_node_ptr))
    #define FREE_NODE_PTR(var)          free(var)
#endif

static inline void _sl_node_init(skiplist_node *node,
                                 size_t top_layer)
{
    if (top_layer > UINT8_MAX) {
        top_layer = UINT8_MAX;
    }

    bool bool_val = false;
    ATM_STORE(node->is_fully_linked, bool_val);
    ATM_STORE(node->being_modified, bool_val);
    ATM_STORE(node->removed, bool_val);

    if (node->top_layer != top_layer ||
        node->next == NULL) {

        node->top_layer = top_layer;

        if (node->next) {
            FREE_NODE_PTR(node->next);
        }
        ALLOC_NODE_PTR(node->next, top_layer+1);
    }
}

void skiplist_init(skiplist_raw *slist,
                   skiplist_cmp_t *cmp_func) {

    slist->cmp_func = NULL;
    slist->aux = NULL;

    // fanout 4 + layer 12: 4^12 ~= upto 17M items under O(lg n) complexity.
    // for +17M items, complexity will grow linearly: O(k lg n).
    slist->fanout = 4;
    slist->max_layer = 12;

    skiplist_init_node(&slist->head);
    skiplist_init_node(&slist->tail);

    _sl_node_init(&slist->head, slist->max_layer);
    _sl_node_init(&slist->tail, slist->max_layer);

    size_t layer;
    for (layer = 0; layer < slist->max_layer; ++layer) {
        slist->head.next[layer] = &slist->tail;
        slist->tail.next[layer] = NULL;
    }

    bool bool_val = true;
    ATM_STORE(slist->head.is_fully_linked, bool_val);
    ATM_STORE(slist->tail.is_fully_linked, bool_val);
    slist->cmp_func = cmp_func;
}

void skiplist_free(skiplist_raw *slist)
{
    skiplist_free_node(&slist->head);
    skiplist_free_node(&slist->tail);
}

void skiplist_init_node(skiplist_node *node)
{
    node->next = NULL;

    bool bool_val = false;
    ATM_STORE(node->is_fully_linked, bool_val);
    ATM_STORE(node->being_modified, bool_val);
    ATM_STORE(node->removed, bool_val);

    node->top_layer = 0;
}

void skiplist_free_node(skiplist_node *node)
{
    FREE_NODE_PTR(node->next);
}

skiplist_raw_config skiplist_get_default_config()
{
    skiplist_raw_config ret;
    ret.fanout = 4;
    ret.maxLayer = 12;
    ret.aux = NULL;
    return ret;
}

skiplist_raw_config skiplist_get_config(skiplist_raw *slist)
{
    skiplist_raw_config ret;
    ret.fanout = slist->fanout;
    ret.maxLayer = slist->max_layer;
    ret.aux = slist->aux;
    return ret;
}

void skiplist_set_config(skiplist_raw *slist,
                         skiplist_raw_config config)
{
    slist->fanout = config.fanout;
    slist->max_layer = config.maxLayer;
    slist->aux = config.aux;
}

static inline int _sl_cmp(skiplist_raw *slist,
                          skiplist_node *a,
                          skiplist_node *b)
{
    if (a == b) {
        return 0;
    }
    if (a == &slist->head ||
        b == &slist->tail) {
        return -1;
    }
    if (a == &slist->tail ||
        b == &slist->head) {
        return 1;
    }
    return slist->cmp_func(a, b, slist->aux);
}

static inline bool _sl_valid_node(skiplist_node *node) {
    bool removed = false;
    bool is_fully_linked = false;

    ATM_LOAD(node->removed, removed);
    ATM_LOAD(node->is_fully_linked, is_fully_linked);

    return !removed && is_fully_linked;
}

static inline skiplist_node* _sl_next(skiplist_raw *slist,
                                     skiplist_node *cur_node,
                                     int layer)
{
    skiplist_node *next_node = NULL;
    ATM_LOAD(cur_node->next[layer], next_node);
    while ( next_node && !_sl_valid_node(next_node) ) {
        ATM_LOAD(next_node->next[layer], next_node);
    }
    return next_node;
}

static inline size_t _sl_decide_top_layer(skiplist_raw *slist)
{
    size_t layer = 0;
    while (layer+1 < slist->max_layer) {
        // coin filp
        if (rand() % slist->fanout == 0) {
            // grow: 1/fanout probability
            layer++;
        } else {
            // stop: 1 - 1/fanout probability
            break;
        }
    }
    return layer;
}

static inline void _sl_clr_flags(skiplist_node** node_arr,
                                 int start_layer,
                                 int top_layer)
{
    int layer;
    for (layer = start_layer; layer <= top_layer; ++layer) {
        if ( layer == top_layer ||
             node_arr[layer] != node_arr[layer+1] ) {

            bool being_modified = false;
            ATM_LOAD(node_arr[layer]->being_modified, being_modified);
            __SLD_ASSERT(being_modified == true);
            (void)being_modified;

            bool bool_val = false;
            ATM_STORE(node_arr[layer]->being_modified, bool_val);
        }
    }
}

static inline bool _sl_valid_prev_next(skiplist_node *prev,
                                       skiplist_node *next) {
    return _sl_valid_node(prev) && _sl_valid_node(next);
}

void skiplist_insert(skiplist_raw *slist,
                     skiplist_node *node)
{
    int top_layer = _sl_decide_top_layer(slist);
    bool bool_val = true;

    // init node before insertion
    _sl_node_init(node, top_layer);

    skiplist_node* prevs[SKIPLIST_MAX_LAYER];
    skiplist_node* nexts[SKIPLIST_MAX_LAYER];

insert_retry:
    // in pure C, a label can only be part of a stmt.
    (void)top_layer;

    int cmp = 0;
    int cur_layer = 0;
    int layer;
    skiplist_node *cur_node = &slist->head;

    for (cur_layer = slist->max_layer-1; cur_layer >= 0; --cur_layer) {
        do {
            skiplist_node *next_node = _sl_next(slist, cur_node, cur_layer);
            cmp = _sl_cmp(slist, node, next_node);
            if (cmp > 0) {
                // cur_node < next_node < node
                // => move to next node
                cur_node = next_node;
                continue;
            }
            // otherwise: cur_node < node <= next_node

            if (cur_layer <= top_layer) {
                prevs[cur_layer] = cur_node;
                nexts[cur_layer] = next_node;

                // both 'prev' and 'next' should be fully linked before
                // insertion, and no other thread should not modify 'prev'
                // at the same time.

                int error_code = 0;
                int locked_layer = cur_layer + 1;

                // check if prev node is duplicated with upper layer
                if (cur_layer < top_layer &&
                    prevs[cur_layer] == prevs[cur_layer+1]) {
                    // duplicate
                    // => which means that 'being_modified' flag is already true
                    // => do nothing
                } else {
                    bool expected = false;
                    bool_val = true;
                    if (ATM_CAS(prevs[cur_layer]->being_modified,
                                expected, bool_val)) {
                        locked_layer = cur_layer;
                    } else {
                        error_code = -1;
                    }
                }

                if (error_code == 0 &&
                    !_sl_valid_prev_next(prevs[cur_layer], nexts[cur_layer])) {
                    error_code = -2;
                }

                if (error_code != 0) {
                    __SLD_RT_INS(error_code, node, top_layer, cur_layer);
                    _sl_clr_flags(prevs, locked_layer, top_layer);
                    goto insert_retry;
                }

                // set current node's pointers
                ATM_STORE(node->next[cur_layer], nexts[cur_layer]);

                if (_sl_next(slist, cur_node, cur_layer) != next_node) {
                    __SLD_NC_INS(cur_node, next_node, top_layer, cur_layer);
                    // clear including the current layer
                    // as we already set modification flag above.
                    _sl_clr_flags(prevs, cur_layer, top_layer);
                    goto insert_retry;
                }
            }

            if (cur_layer) {
                // non-bottom layer => go down
                break;
            }

            // bottom layer => insertion succeeded
            // change prev/next nodes' prev/next pointers from 0 ~ top_layer
            for (layer = 0; layer <= top_layer; ++layer) {
                ATM_STORE(prevs[layer]->next[layer], node);
            }

            // now this node is fully linked
            bool_val = true;
            ATM_STORE(node->is_fully_linked, bool_val);

            // modification is done for all layers
            _sl_clr_flags(prevs, 0, top_layer);
            return;

        } while (cur_node != &slist->tail);
    }
}

typedef enum {
    SM = -2,
    SMEQ = -1,
    EQ = 0,
    GTEQ = 1,
    GT = 2
} _sl_find_mode;

static inline skiplist_node* _sl_find(skiplist_raw *slist,
                                      skiplist_node *query,
                                      _sl_find_mode mode)
{
    // mode:
    //  SM   -2: smaller
    //  SMEQ -1: smaller or equal
    //  EQ    0: equal
    //  GTEQ  1: greater or equal
    //  GT    2: greater
    int cmp = 0;
    int cur_layer = 0;
    skiplist_node *cur_node = &slist->head;

    for (cur_layer = slist->max_layer-1; cur_layer >= 0; --cur_layer) {
        do {
            skiplist_node *next_node = _sl_next(slist, cur_node, cur_layer);
            cmp = _sl_cmp(slist, query, next_node);
            if (cmp > 0) {
                // cur_node < next_node < query
                // => move to next node
                cur_node = next_node;
                continue;
            } else if (-1 <= mode && mode <= 1 && cmp == 0) {
                // cur_node < query == next_node .. return
                return next_node;
            }

            // otherwise: cur_node < query < next_node
            if (cur_layer) {
                // non-bottom layer => go down
                break;
            }

            // bottom layer
            if (mode < 0 && cur_node != &slist->head) {
                // smaller mode
                return cur_node;
            } else if (mode > 0 && next_node != &slist->tail) {
                // greater mode
                return next_node;
            }
            // otherwise: exact match mode OR not found
            return NULL;

        } while (cur_node != &slist->tail);
    }

    return NULL;
}

skiplist_node* skiplist_find(skiplist_raw *slist,
                             skiplist_node *query)
{
    return _sl_find(slist, query, EQ);
}

skiplist_node* skiplist_find_smaller_or_equal(skiplist_raw *slist,
                                              skiplist_node *query)
{
    return _sl_find(slist, query, SMEQ);
}

skiplist_node* skiplist_find_greater_or_equal(skiplist_raw *slist,
                                              skiplist_node *query)
{
    return _sl_find(slist, query, GTEQ);
}

int skiplist_erase_node(skiplist_raw *slist,
                        skiplist_node *node)
{
    int top_layer = node->top_layer;
    bool bool_val = true;
    bool removed = false;
    bool is_fully_linked = false;

    ATM_LOAD(node->removed, removed);
    if (removed) {
        // already removed
        return -1;
    }

    skiplist_node* prevs[SKIPLIST_MAX_LAYER];
    skiplist_node* nexts[SKIPLIST_MAX_LAYER];

    bool expected = false;
    bool_val = true;
    if (!ATM_CAS(node->being_modified, expected, bool_val)) {
        // already being modified .. fail
        __SLD_BM(node);
        return -2;
    }

    // clear removed flag first, so that reader cannot read this node.
    bool_val = true;
    ATM_STORE(node->removed, bool_val);

erase_node_retry:
    ATM_LOAD(node->is_fully_linked, is_fully_linked);
    if (!is_fully_linked) {
        // already unlinked .. remove is done by other thread
        return -3;
    }

    int cmp = 0;
    int cur_layer = slist->max_layer - 1;
    skiplist_node *cur_node = &slist->head;

    for (; cur_layer >= 0; --cur_layer) {
        do {
            skiplist_node *next_node = _sl_next(slist, cur_node, cur_layer);

            cmp = _sl_cmp(slist, node, next_node);
            if (cmp > 0) {
                // cur_node < next_node < node
                // => move to next node
                cur_node = next_node;
                continue;
            }
            // otherwise: cur_node < node <= next_node

            if (cur_layer <= top_layer) {
                prevs[cur_layer] = cur_node;
                // note: 'next_node' and 'node' should not be the node,
                //       as 'removed' flag is already set.
                __SLD_ASSERT(next_node != node);
                nexts[cur_layer] = next_node;

                // check if prev node duplicates with upper layer
                int error_code = 0;
                int locked_layer = cur_layer + 1;
                if (cur_layer < top_layer &&
                    prevs[cur_layer] == prevs[cur_layer+1]) {
                    // duplicate with upper layer
                    // => which means that 'being_modified' flag is already true
                    // => do nothing.
                } else {
                    expected = false;
                    bool_val = true;
                    if (ATM_CAS(prevs[cur_layer]->being_modified,
                                expected, bool_val)) {
                        locked_layer = cur_layer;
                    } else {
                        error_code = -1;
                    }
                }

                if (error_code == 0 &&
                    !_sl_valid_prev_next(prevs[cur_layer], nexts[cur_layer])) {
                    error_code = -2;
                }

                if (error_code != 0) {
                    __SLD_RT_RMV(error_code, node, top_layer, cur_layer);
                    _sl_clr_flags(prevs, locked_layer, top_layer);
                    goto erase_node_retry;
                }

                if (_sl_next(slist, cur_node, cur_layer) != nexts[cur_layer]) {
                    __SLD_NC_RMV(cur_node, nexts[cur_layer], top_layer, cur_layer);
                    _sl_clr_flags(prevs, cur_layer, top_layer);
                    goto erase_node_retry;
                }
            }

            // go down
            break;

        } while (cur_node != &slist->tail);
    }

    // bottom layer => removal succeeded.
    // change prev nodes' next pointer from 0 ~ top_layer
    for (cur_layer = 0; cur_layer <= top_layer; ++cur_layer) {
        ATM_STORE(prevs[cur_layer]->next[cur_layer], nexts[cur_layer]);
    }

    // now this node is unlinked
    bool_val = false;
    ATM_STORE(node->is_fully_linked, bool_val);

    // modification is done for all layers
    _sl_clr_flags(prevs, 0, top_layer);

    bool_val = false;
    ATM_STORE(node->being_modified, bool_val);
    return 0;
}

int skiplist_erase(skiplist_raw *slist,
                   skiplist_node *query)
{
    skiplist_node *found = skiplist_find(slist, query);
    if (!found) {
        // key not found
        return -4;
    }

    int ret = 0;
    do {
        ret = skiplist_erase_node(slist, found);
        // if ret == -2, other thread is accessing the same node
        // at the same time. try again.
    } while (ret == -2);
    return ret;
}

skiplist_node* skiplist_next(skiplist_raw *slist,
                             skiplist_node *node) {
    skiplist_node *next = _sl_next(slist, node, 0);
    if (next == &slist->tail) {
        return NULL;
    }
    return next;
}

skiplist_node* skiplist_prev(skiplist_raw *slist,
                             skiplist_node *node) {
    skiplist_node *prev = _sl_find(slist, node, SM);
    if (prev == &slist->head) {
        return NULL;
    }
    return prev;
}

skiplist_node* skiplist_begin(skiplist_raw *slist) {
    skiplist_node *next = _sl_next(slist, &slist->head, 0);
    if (next == &slist->tail) {
        return NULL;
    }
    return next;
}

skiplist_node* skiplist_end(skiplist_raw *slist) {
    return skiplist_prev(slist, &slist->tail);
}


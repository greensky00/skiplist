/**
 * Copyright (C) 2017-present Jung-Sang Ahn <jungsang.ahn@gmail.com>
 * All rights reserved.
 *
 * https://github.com/greensky00
 *
 * Skiplist
 * Version: 0.2.9
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

#ifndef _JSAHN_SKIPLIST_H
#define _JSAHN_SKIPLIST_H (1)

#include <stddef.h>
#include <stdint.h>

#define SKIPLIST_MAX_LAYER (64)

struct _skiplist_node;

//#define _STL_ATOMIC (1)
#ifdef __APPLE__
    #define _STL_ATOMIC (1)
#endif
#if defined(_STL_ATOMIC) && defined(__cplusplus)
    #include <atomic>
    typedef std::atomic<_skiplist_node*>   atm_node_ptr;
    typedef std::atomic<bool>              atm_bool;
    typedef std::atomic<uint8_t>           atm_uint8_t;
    typedef std::atomic<uint16_t>          atm_uint16_t;
    typedef std::atomic<uint32_t>          atm_uint32_t;
#else
    typedef struct _skiplist_node*         atm_node_ptr;
    typedef uint8_t                        atm_bool;
    typedef uint8_t                        atm_uint8_t;
    typedef uint16_t                       atm_uint16_t;
    typedef uint32_t                       atm_uint32_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _skiplist_node {
    atm_node_ptr *next;
    atm_bool is_fully_linked;
    atm_bool being_modified;
    atm_bool removed;
    uint8_t top_layer; // 0: bottom
    atm_uint16_t ref_count;
    atm_uint32_t accessing_next;
} skiplist_node;

// *a  < *b : return neg
// *a == *b : return 0
// *a  > *b : return pos
typedef int skiplist_cmp_t(skiplist_node *a, skiplist_node *b, void *aux);

typedef struct {
    size_t fanout;
    size_t maxLayer;
    void *aux;
} skiplist_raw_config;

typedef struct {
    skiplist_node head;
    skiplist_node tail;
    skiplist_cmp_t *cmp_func;
    void *aux;
    atm_uint32_t num_entries;
    atm_uint32_t* layer_entries;
    atm_uint8_t top_layer;
    uint8_t fanout;
    uint8_t max_layer;
} skiplist_raw;

#ifndef _get_entry
#define _get_entry(ELEM, STRUCT, MEMBER)                              \
        ((STRUCT *) ((uint8_t *) (ELEM) - offsetof (STRUCT, MEMBER)))
#endif

void skiplist_init(skiplist_raw* slist,
                   skiplist_cmp_t* cmp_func);
void skiplist_free(skiplist_raw* slist);

void skiplist_init_node(skiplist_node* node);
void skiplist_free_node(skiplist_node* node);

size_t skiplist_get_size(skiplist_raw* slist);

skiplist_raw_config skiplist_get_default_config();
skiplist_raw_config skiplist_get_config(skiplist_raw* slist);

void skiplist_set_config(skiplist_raw* slist,
                         skiplist_raw_config config);

int skiplist_insert(skiplist_raw* slist,
                    skiplist_node* node);
int skiplist_insert_nodup(skiplist_raw *slist,
                          skiplist_node *node);

skiplist_node* skiplist_find(skiplist_raw* slist,
                             skiplist_node* query);
skiplist_node* skiplist_find_smaller_or_equal(skiplist_raw* slist,
                                              skiplist_node* query);
skiplist_node* skiplist_find_greater_or_equal(skiplist_raw* slist,
                                              skiplist_node* query);

int skiplist_erase_node_passive(skiplist_raw* slist,
                                skiplist_node* node);
int skiplist_erase_node(skiplist_raw *slist,
                        skiplist_node *node);
int skiplist_erase(skiplist_raw* slist,
                   skiplist_node* query);

int skiplist_is_valid_node(skiplist_node* node);
int skiplist_is_safe_to_free(skiplist_node* node);
void skiplist_wait_for_free(skiplist_node* node);

void skiplist_grab_node(skiplist_node* node);
void skiplist_release_node(skiplist_node* node);

skiplist_node* skiplist_next(skiplist_raw* slist,
                             skiplist_node* node);
skiplist_node* skiplist_prev(skiplist_raw* slist,
                             skiplist_node* node);
skiplist_node* skiplist_begin(skiplist_raw* slist);
skiplist_node* skiplist_end(skiplist_raw* slist);

#ifdef __cplusplus
}
#endif

#endif  // _JSAHN_SKIPLIST_H

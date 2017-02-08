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

#ifndef _JSAHN_SKIPLIST_H
#define _JSAHN_SKIPLIST_H (1)

#include <stddef.h>
#include <stdint.h>

#define SKIPLIST_MAX_LAYER (256)

struct _skiplist_node;

#define _STL_ATOMIC (1)
#if defined(_STL_ATOMIC) && defined(__cplusplus)
    #include <atomic>
    typedef std::atomic<_skiplist_node*> atm_node_ptr;
    typedef std::atomic<bool> atm_bool;
#else
    typedef struct _skiplist_node* atm_node_ptr;
    typedef uint8_t atm_bool;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void skiplist_free_node(struct _skiplist_node *node);

typedef struct _skiplist_node {
    atm_node_ptr *next;
    atm_bool isFullyLinked;
    atm_bool beingModified;
    atm_bool removed;
    uint8_t topLayer; // 0: bottom
} SkiplistNode;

// *a  < *b : return neg
// *a == *b : return 0
// *a  > *b : return pos
typedef int skiplist_cmp_t(SkiplistNode *a, SkiplistNode *b, void *aux);

typedef struct {
    size_t fanout;
    size_t maxLayer;
    void *aux;
} SkiplistRawConfig;

typedef struct {
    SkiplistNode head;
    SkiplistNode tail;
    skiplist_cmp_t *cmpFunc;
    void *aux;
    uint8_t fanout;
    uint8_t maxLayer;
} SkiplistRaw;

#ifndef _get_entry
#define _get_entry(ELEM, STRUCT, MEMBER)                              \
        ((STRUCT *) ((uint8_t *) (ELEM) - offsetof (STRUCT, MEMBER)))
#endif

void skiplist_init(SkiplistRaw *slist,
                   skiplist_cmp_t *cmp_func);

void skiplist_free(SkiplistRaw *slist);

void skiplist_init_node(SkiplistNode *node);

void skiplist_free_node(SkiplistNode *node);

SkiplistRawConfig skiplist_get_default_config();

SkiplistRawConfig skiplist_get_config(SkiplistRaw *slist);

void skiplist_set_config(SkiplistRaw *slist,
                         SkiplistRawConfig config);

void skiplist_insert(SkiplistRaw *slist,
                     SkiplistNode *node);

SkiplistNode* skiplist_find(SkiplistRaw *slist,
                            SkiplistNode *query);

SkiplistNode* skiplist_find_smaller(SkiplistRaw *slist,
                                    SkiplistNode *query);

int skiplist_erase_node(SkiplistRaw *slist,
                        SkiplistNode *node);

int skiplist_erase(SkiplistRaw *slist,
                   SkiplistNode *query);

SkiplistNode* skiplist_next(SkiplistRaw *slist,
                            SkiplistNode *node);

SkiplistNode* skiplist_prev(SkiplistRaw *slist,
                            SkiplistNode *node);

SkiplistNode* skiplist_begin(SkiplistRaw *slist);

SkiplistNode* skiplist_end(SkiplistRaw *slist);

#ifdef __cplusplus
}
#endif

#endif  // _JSAHN_SKIPLIST_H

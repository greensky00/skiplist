/**
 * Copyright (C) 2017 Jung-Sang Ahn <jungsang.ahn@gmail.com>
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

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <atomic>

struct SkiplistNode {
    SkiplistNode() :
        next(nullptr),
        isFullyLinked(false),
        beingModified(false),
        removed(false),
        topLayer(0)
    { }

    ~SkiplistNode() {
        delete[] next;
    }

    std::atomic<SkiplistNode*>* next;
    std::atomic<bool> isFullyLinked;
    std::atomic<bool> beingModified;
    std::atomic<bool> removed;
    uint8_t topLayer; // 0: bottom
};

#define SKIPLIST_MAX_LAYER (256)

// *a  < *b : return neg
// *a == *b : return 0
// *a  > *b : return pos
typedef int skiplist_cmp_t(SkiplistNode *a, SkiplistNode *b, void *aux);

struct SkiplistRawConfig {
    SkiplistRawConfig() :
        fanout(4),
        maxLayer(12),
        aux(nullptr)
        { }

    size_t fanout;
    size_t maxLayer;
    void *aux;
};

struct SkiplistRaw {
    // fanout 4 + layer 12: 4^12 ~= upto 17M items under O(lg n) complexity.
    // for +17M items, complexity will grow linearly: O(k lg n).
    SkiplistRaw() :
        cmpFunc(nullptr),
        aux(nullptr),
        fanout(4),
        maxLayer(12)
        { }

    SkiplistNode head;
    SkiplistNode tail;
    skiplist_cmp_t *cmpFunc;
    void *aux;
    uint8_t fanout;
    uint8_t maxLayer;
};

#ifndef _get_entry
#define _get_entry(ELEM, STRUCT, MEMBER)                              \
        ((STRUCT *) ((uint8_t *) (ELEM) - offsetof (STRUCT, MEMBER)))
#endif


void skiplist_init(SkiplistRaw *slist,
                   skiplist_cmp_t *cmp_func);

void skiplist_set_config(SkiplistRaw *slist,
                         SkiplistRawConfig config);

SkiplistRawConfig skiplist_get_config(SkiplistRaw *slist);

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


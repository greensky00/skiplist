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

#include <stdio.h>
#include <assert.h>

#include "skiplist.h"

struct dbg_node {
    SkiplistNode snode;
    int value;
};

#define __SLD_RT_INS(e, n, t, c) __sld_rt_ins(e, n, t, c)
#define __SLD_NC_INS(n, nn, t, c) __sld_nc_ins(n, nn, t, c)
#define __SLD_RT_RMV(e, n, t, c) __sld_rt_rmv(e, n, t, c)
#define __SLD_NC_RMV(n, nn, t, c) __sld_nc_rmv(n, nn, t, c)
#define __SLD_BM(n) __sld_bm(n)
#define __SLD_ASSERT(cond) assert(cond)

inline void __sld_rt_ins(int error_code,
                         SkiplistNode *node,
                         int top_layer,
                         int cur_layer)
{
    dbg_node *ddd = _get_entry(node, dbg_node, snode);
    printf("[INS] retry (code %d) "
           "%lx (top %d, cur %d) %d\n",
           error_code, (uint64_t)node,
           top_layer, cur_layer, ddd->value);
}

inline void __sld_nc_ins(SkiplistNode *node,
                         SkiplistNode *next_node,
                         int top_layer,
                         int cur_layer)
{
    dbg_node *ddd = _get_entry(node, dbg_node, snode);
    dbg_node *ddd_next = _get_entry(next_node, dbg_node, snode);

    printf("[INS] next node changed, "
           "%lx %lx (top %d, cur %d) %d %d\n",
           (uint64_t)node, (uint64_t)next_node,
           top_layer, cur_layer, ddd->value, ddd_next->value);
}

inline void __sld_rt_rmv(int error_code,
                         SkiplistNode *node,
                         int top_layer,
                         int cur_layer)
{
    dbg_node *ddd = _get_entry(node, dbg_node, snode);
    printf("[RMV] retry (code %d) "
           "%lx (top %d, cur %d) %d\n",
           error_code, (uint64_t)node,
           top_layer, cur_layer, ddd->value);
}

inline void __sld_nc_rmv(SkiplistNode *node,
                         SkiplistNode *next_node,
                         int top_layer,
                         int cur_layer)
{
    dbg_node *ddd = _get_entry(node, dbg_node, snode);
    dbg_node *ddd_next = _get_entry(next_node, dbg_node, snode);

    printf("[RMV] next node changed, "
           "%lx %lx (top %d, cur %d) %d %d\n",
           (uint64_t)node, (uint64_t)next_node,
           top_layer, cur_layer, ddd->value, ddd_next->value);
}

inline void __sld_bm(SkiplistNode *node) {
    dbg_node *ddd = _get_entry(node, dbg_node, snode);
    printf("[RMV] node is being modified %d\n", ddd->value);
}


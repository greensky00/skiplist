/**
 * Copyright (C) 2017-present Jung-Sang Ahn <jungsang.ahn@gmail.com>
 * All rights reserved.
 *
 * https://github.com/greensky00
 *
 * Skiplist
 * Version: 0.2.5
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

#include <assert.h>
#include <stdio.h>

#include "skiplist.h"

struct dbg_node {
    skiplist_node snode;
    int value;
};

#if __SL_DEBUG >= 1
    #undef __SLD_ASSERT
    #undef __SLD_
    #define __SLD_ASSERT(cond) assert(cond)
    #define __SLD_(b) b
#endif
#if __SL_DEBUG >= 2
    #undef __SLD_P
    #define __SLD_P(args...) printf(args)
#endif
#if __SL_DEBUG >= 3
    #undef __SLD_RT_INS
    #undef __SLD_NC_INS
    #undef __SLD_RT_RMV
    #undef __SLD_NC_RMV
    #undef __SLD_BM
    #define __SLD_RT_INS(e, n, t, c) __sld_rt_ins(e, n, t, c)
    #define __SLD_NC_INS(n, nn, t, c) __sld_nc_ins(n, nn, t, c)
    #define __SLD_RT_RMV(e, n, t, c) __sld_rt_rmv(e, n, t, c)
    #define __SLD_NC_RMV(n, nn, t, c) __sld_nc_rmv(n, nn, t, c)
    #define __SLD_BM(n) __sld_bm(n)
#endif
#if __SL_DEBUG >= 4
    #error "unknown debugging level"
#endif


inline void __sld_rt_ins(int error_code,
                         skiplist_node *node,
                         int top_layer,
                         int cur_layer)
{
    dbg_node *ddd = _get_entry(node, dbg_node, snode);
    printf("[INS] retry (code %d) "
           "%p (top %d, cur %d) %d\n",
           error_code, node,
           top_layer, cur_layer, ddd->value);
}

inline void __sld_nc_ins(skiplist_node *node,
                         skiplist_node *next_node,
                         int top_layer,
                         int cur_layer)
{
    dbg_node *ddd = _get_entry(node, dbg_node, snode);
    dbg_node *ddd_next = _get_entry(next_node, dbg_node, snode);

    printf("[INS] next node changed, "
           "%p %p (top %d, cur %d) %d %d\n",
           node, next_node,
           top_layer, cur_layer, ddd->value, ddd_next->value);
}

inline void __sld_rt_rmv(int error_code,
                         skiplist_node *node,
                         int top_layer,
                         int cur_layer)
{
    dbg_node *ddd = _get_entry(node, dbg_node, snode);
    printf("[RMV] retry (code %d) "
           "%p (top %d, cur %d) %d\n",
           error_code, node,
           top_layer, cur_layer, ddd->value);
}

inline void __sld_nc_rmv(skiplist_node *node,
                         skiplist_node *next_node,
                         int top_layer,
                         int cur_layer)
{
    dbg_node *ddd = _get_entry(node, dbg_node, snode);
    dbg_node *ddd_next = _get_entry(next_node, dbg_node, snode);

    printf("[RMV] next node changed, "
           "%p %p (top %d, cur %d) %d %d\n",
           node, next_node,
           top_layer, cur_layer, ddd->value, ddd_next->value);
}

inline void __sld_bm(skiplist_node *node) {
    dbg_node *ddd = _get_entry(node, dbg_node, snode);
    printf("[RMV] node is being modified %d\n", ddd->value);
}


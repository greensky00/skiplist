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
#include <stdlib.h>
#include <vector>
#include <chrono>
#include <ctime>
#include <thread>

#include <assert.h>

#include "skiplist.h"

struct IntNode {
    SkiplistNode snode;
    int value;
};

int _cmp_IntNode(SkiplistNode *a, SkiplistNode *b, void *aux)
{
    IntNode *aa, *bb;
    aa = _get_entry(a, IntNode, snode);
    bb = _get_entry(b, IntNode, snode);
    if (aa->value < bb->value) {
        return -1;
    } else if (aa->value == bb->value) {
        return 0;
    } else {
        return 1;
    }
}

void basic_insert_and_erase()
{
    SkiplistRaw list;
    skiplist_init(&list, _cmp_IntNode);

    int i, j, temp;
    int n = 16;
    std::vector<int> key(n);
    std::vector<IntNode> arr(n);

    // key assign
    for (i=0; i<n; ++i) {
        key[i] = i;
    }
    // shuffle
    srand(0xabcd);
    for (i=0; i<n; ++i) {
        j = rand() % n;
        temp = key[i];
        key[i] = key[j];
        key[j] = temp;
    }

    // random insert
    for (i=0; i<n; ++i) {
        arr[i].value = key[i];
        skiplist_insert(&list, &arr[i].snode);
    }

    // forward iteration
    int count = 0;
    SkiplistNode *cur = skiplist_begin(&list);
    while (cur) {
        IntNode *node = _get_entry(cur, IntNode, snode);
        assert(node->value == count);
        cur = skiplist_next(&list, cur);
        count++;
    }
    assert(count == n);

    // backward iteration
    count = n;
    cur = skiplist_end(&list);
    while (cur) {
        count--;
        IntNode *node = _get_entry(cur, IntNode, snode);
        assert(node->value == count);
        cur = skiplist_prev(&list, cur);
    }
    assert(count == 0);

    // remove even numbers
    IntNode query;
    for (i=0; i<n; i+=2) {
        query.value = i;
        cur = skiplist_find(&list, &query.snode);
        assert(cur);
        int ret_value = skiplist_erase_node(&list, cur);
        assert(ret_value == 0);
    }

    // forward iteration
    count = 0;
    cur = skiplist_begin(&list);
    while (cur) {
        IntNode *node = _get_entry(cur, IntNode, snode);
        assert(node->value == count*2 + 1);
        cur = skiplist_next(&list, cur);
        count++;
    }
    assert(count == n/2);

    // backward iteration
    count = n/2;
    cur = skiplist_end(&list);
    while (cur) {
        count--;
        IntNode *node = _get_entry(cur, IntNode, snode);
        assert(node->value == count*2 + 1);
        cur = skiplist_prev(&list, cur);
    }
    assert(count == 0);
}

void find_test()
{
    SkiplistRaw list;
    skiplist_init(&list, _cmp_IntNode);

    int i, j, temp;
    int n = 10000;
    std::vector<int> key(n);
    std::vector<IntNode> arr(n);
    // assign
    for (i=0; i<n; ++i) {
        key[i] = i*10;
    }
    // shuffle
    srand(0x1111);
    for (i=0; i<n; ++i) {
        j = rand() % n;
        temp = key[i];
        key[i] = key[j];
        key[j] = temp;
    }

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    for (i=0; i<n; ++i) {
        arr[i].value = key[i];
        skiplist_insert(&list, &arr[i].snode);
    }
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    printf("insert %.4f (%.1f ops/sec)\n",
           elapsed_seconds.count(), n/elapsed_seconds.count());

    // find exact match key
    IntNode query, *item;
    SkiplistNode *ret;

    start = std::chrono::system_clock::now();
    for (i=0; i<n; ++i) {
        query.value = i*10;
        ret = skiplist_find(&list, &query.snode);
        assert(ret);
        item = _get_entry(ret, IntNode, snode);
        assert(item->value == query.value);
    }
    end = std::chrono::system_clock::now();
    elapsed_seconds = end-start;
    printf("find (existing key) done: %.4f (%.1f ops/sec)\n",
           elapsed_seconds.count(), n/elapsed_seconds.count());

    // find non-existing key
    start = std::chrono::system_clock::now();
    for (i=0; i<n; ++i) {
        query.value = i*10 + 1;
        ret = skiplist_find(&list, &query.snode);
        assert(ret == nullptr);
    }
    end = std::chrono::system_clock::now();
    elapsed_seconds = end-start;
    printf("find (non-existing key) done: %.4f (%.1f ops/sec)\n",
           elapsed_seconds.count(), n/elapsed_seconds.count());
}

struct writer_T1_args {
    SkiplistRaw* list;
    std::vector<int>* key;
    std::vector<IntNode>* arr;
    int range_begin;
    int range_end;
    int id;
    int n_keys;
};

void* writer_T1(void *voidargs)
{
    writer_T1_args *args = (writer_T1_args*)voidargs;
    int i;
    for (i=args->range_begin; i<=args->range_end; ++i) {
        IntNode& node = args->arr->at(i);
        skiplist_insert(args->list, &node.snode);
    }

    return NULL;
}

void concurrent_write_test()
{
    SkiplistRaw list;
    skiplist_init(&list, _cmp_IntNode);

    int i, j, temp;
    int n = 1000000;
    std::vector<int> key(n);
    std::vector<IntNode> arr(n);
    // assign
    for (i=0; i<n; ++i) {
        key[i] = i;
    }
    // shuffle
    srand(0x1111);
    for (i=0; i<n; ++i) {
        j = rand() % n;
        temp = key[i];
        key[i] = key[j];
        key[j] = temp;
    }

    for (i=0; i<n; ++i) {
        arr[i].value = key[i];
    }

    int n_threads = 8;
    int n_keys_per_thread = n / n_threads;

    std::vector<std::thread> t_hdl(n_threads);
    std::vector<writer_T1_args> args(n_threads);

    for (i=0; i<n_threads; ++i) {
        args[i].list = &list;
        args[i].key = &key;
        args[i].arr = &arr;
        args[i].range_begin = i*n_keys_per_thread;
        args[i].range_end = (i+1)*n_keys_per_thread - 1;
        args[i].id = i;
        args[i].n_keys = n;

        t_hdl[i] = std::thread(writer_T1, &args[i]);
    }

    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::chrono::duration<double> elapsed_seconds;
    start = std::chrono::system_clock::now();

    for (i=0; i<n_threads; ++i){
        t_hdl[i].join();
    }

    end = std::chrono::system_clock::now();
    elapsed_seconds = end-start;

    printf("insert %.4f (%.1f ops/sec)\n",
           elapsed_seconds.count(), n/elapsed_seconds.count());

    // forward iteration
    start = std::chrono::system_clock::now();

    int count = 0;
    bool corruption = false;
    SkiplistNode *cur = skiplist_begin(&list);
    while (cur) {
        IntNode *node = _get_entry(cur, IntNode, snode);
        if (node->value != count) {
            SkiplistNode *missing = &arr[count].snode;
            printf("idx %d is missing, %lx\n", count, (uint64_t)missing);

            SkiplistNode *prev = skiplist_prev(&list, missing);
            SkiplistNode *next = skiplist_next(&list, missing);
            IntNode *prev_node = _get_entry(prev, IntNode, snode);
            IntNode *next_node = _get_entry(next, IntNode, snode);
            printf("%d %d\n", prev_node->value, next_node->value);

            count = node->value;
            corruption = true;
        }
        assert(node->value == count);
        cur = skiplist_next(&list, cur);
        count++;
    }
    assert(count == n);
    assert(!corruption);

    end = std::chrono::system_clock::now();
    elapsed_seconds = end-start;

    printf("iteration %.4f (%.1f ops/sec)\n",
           elapsed_seconds.count(), n/elapsed_seconds.count());
}

void* eraser_T2(void *voidargs)
{
    writer_T1_args *args = (writer_T1_args*)voidargs;
    int i;
    for (i=args->range_begin; i<=args->range_end; ++i) {
        IntNode query;
        query.value = args->key->at(i);
        int ret = skiplist_erase(args->list, &query.snode);
        (void)ret;
    }

    return NULL;
}

void concurrent_write_erase_test()
{
    SkiplistRaw list;
    skiplist_init(&list, _cmp_IntNode);

    int i, j, temp;
    int n = 1000000;
    std::vector<int> key_add(n);
    std::vector<int> key_del(n);
    std::vector<IntNode> arr_add(n);
    std::vector<IntNode> arr_del(n);
    std::vector<IntNode*> arr_add_dbgref(n);

    // initial insert
    for (i=0; i<n; ++i) {
        // 0, 10, 20, 30 ...
        key_del[i] = i * 10;
        arr_del[i].value = key_del[i];
        skiplist_insert(&list, &arr_del[i].snode);
    }
    printf("initial load done\n");

    // assign keys to add
    for (i=0; i<n; ++i) {
        // 5, 15, 25, 35, ...
        key_add[i] = i*10 + 5;
    }

    // shuffle keys to add
    srand(0x1111);
    for (i=0; i<n; ++i) {
        j = rand() % n;
        temp = key_add[i];
        key_add[i] = key_add[j];
        key_add[j] = temp;
    }
    for (i=0; i<n; ++i) {
        int ori_idx = (key_add[i] - 5) / 10;
        arr_add[i].value = key_add[i];
        arr_add_dbgref[ori_idx] = &arr_add[i];
    }

    // also shuffle keys to delete
    for (i=0; i<n; ++i) {
        j = rand() % n;
        temp = key_del[i];
        key_del[i] = key_del[j];
        key_del[j] = temp;
    }

    int n_threads_add = 4;
    int n_threads_del = 4;

    int n_keys_per_thread_add = n / n_threads_add;
    int n_keys_per_thread_del = n / n_threads_del;

    std::vector<std::thread> t_hdl_add(n_threads_add);
    std::vector<writer_T1_args> args_add(n_threads_add);

    for (i=0; i<n_threads_add; ++i) {
        args_add[i].list = &list;
        args_add[i].key = &key_add;
        args_add[i].arr = &arr_add;
        args_add[i].range_begin = i*n_keys_per_thread_add;
        args_add[i].range_end = (i+1)*n_keys_per_thread_add - 1;
        args_add[i].id = i;
        args_add[i].n_keys = n;

        t_hdl_add[i] = std::thread(writer_T1, &args_add[i]);
    }

    std::vector<std::thread> t_hdl_del(n_threads_del);
    std::vector<writer_T1_args> args_del(n_threads_del);

    for (i=0; i<n_threads_del; ++i) {
        args_del[i].list = &list;
        args_del[i].key = &key_del;
        args_del[i].arr = &arr_del;
        args_del[i].range_begin = i*n_keys_per_thread_del;
        args_del[i].range_end = (i+1)*n_keys_per_thread_del - 1;
        args_del[i].id = i;
        args_del[i].n_keys = n;

        t_hdl_del[i] = std::thread(eraser_T2, &args_del[i]);
    }

    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::chrono::duration<double> elapsed_seconds;
    start = std::chrono::system_clock::now();

    for (i=0; i<n_threads_add; ++i){
        t_hdl_add[i].join();
    }
    for (i=0; i<n_threads_del; ++i){
        t_hdl_del[i].join();
    }

    end = std::chrono::system_clock::now();
    elapsed_seconds = end-start;

    printf("mutation total %.4f (%.1f ops/sec)\n",
           elapsed_seconds.count(), (n*2)/elapsed_seconds.count());

    // forward iteration
    start = std::chrono::system_clock::now();

    int count = 0;
    bool corruption = false;
    SkiplistNode *cur = skiplist_begin(&list);
    std::vector<SkiplistNode*> dbg_node(n);
    std::vector<IntNode*> dbg_int(n);

    while (cur) {
        IntNode *node = _get_entry(cur, IntNode, snode);
        dbg_node[count] = cur;
        dbg_int[count] = node;
        // 5, 15, 25, 35 ...
        int idx = count * 10 + 5;
        if (node->value != idx) {
            SkiplistNode *missing = &arr_add_dbgref[count]->snode;
            printf("count %d, idx %d is missing %lx\n", count, idx, (uint64_t)missing);

            SkiplistNode *prev = skiplist_prev(&list, missing);
            SkiplistNode *next = skiplist_next(&list, missing);
            IntNode *prev_node = _get_entry(prev, IntNode, snode);
            IntNode *next_node = _get_entry(next, IntNode, snode);
            printf("%d %d\n", prev_node->value, next_node->value);

            corruption = true;
        }
        assert(node->value == idx);
        cur = skiplist_next(&list, cur);
        count++;
    }
    assert(count == n);
    assert(!corruption);

    end = std::chrono::system_clock::now();
    elapsed_seconds = end-start;

    printf("iteration %.4f (%.1f ops/sec)\n",
           elapsed_seconds.count(), n/elapsed_seconds.count());
}


int main() {
    basic_insert_and_erase();
    find_test();
    concurrent_write_test();
    concurrent_write_erase_test();

    return 0;
}



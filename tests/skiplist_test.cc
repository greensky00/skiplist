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

#include "skiplist.h"

#include "test_common.h"

#include <vector>
#include <chrono>
#include <ctime>
#include <thread>
#include <set>
#include <mutex>

#include <stdio.h>
#include <stdlib.h>

struct IntNode {
    IntNode() {
        skiplist_init_node(&snode);
    }
    ~IntNode() {
        skiplist_free_node(&snode);
    }

    skiplist_node snode;
    int value;
};

int _cmp_IntNode(skiplist_node *a, skiplist_node *b, void *aux)
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

int basic_insert_and_erase()
{
    skiplist_raw list;
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
    skiplist_node *cur = skiplist_begin(&list);
    while (cur) {
        IntNode *node = _get_entry(cur, IntNode, snode);
        CHK_EQ(count, node->value);
        cur = skiplist_next(&list, cur);
        count++;
    }
    CHK_EQ(n, count);

    // backward iteration
    count = n;
    cur = skiplist_end(&list);
    while (cur) {
        count--;
        IntNode *node = _get_entry(cur, IntNode, snode);
        CHK_EQ(count, node->value);
        cur = skiplist_prev(&list, cur);
    }
    CHK_EQ(0, count);

    // remove even numbers
    IntNode query;
    for (i=0; i<n; i+=2) {
        query.value = i;
        cur = skiplist_find(&list, &query.snode);
        CHK_NONNULL(cur);
        int ret_value = skiplist_erase_node(&list, cur);
        CHK_EQ(0, ret_value);
    }

    // forward iteration
    count = 0;
    cur = skiplist_begin(&list);
    while (cur) {
        IntNode *node = _get_entry(cur, IntNode, snode);
        CHK_EQ(count*2 + 1, node->value);
        cur = skiplist_next(&list, cur);
        count++;
    }
    CHK_EQ(n/2, count);

    // backward iteration
    count = n/2;
    cur = skiplist_end(&list);
    while (cur) {
        count--;
        IntNode *node = _get_entry(cur, IntNode, snode);
        CHK_EQ(count*2 + 1, node->value);
        cur = skiplist_prev(&list, cur);
    }
    CHK_EQ(0, count);

    skiplist_free(&list);

    return 0;
}

int find_test()
{
    TestSuite::Timer tt;
    TestSuite::appendResultMessage("\n");

    double elapsed_sec = 1;
    char msg[1024];

    skiplist_raw list;
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
    for (i=0; i<n; ++i) {
        j = rand() % n;
        temp = key[i];
        key[i] = key[j];
        key[j] = temp;
    }

    tt.reset();
    for (i=0; i<n; ++i) {
        arr[i].value = key[i];
        skiplist_insert(&list, &arr[i].snode);
    }
    elapsed_sec = tt.getTimeUs() / 1000000.0;
    sprintf(msg, "insert %.4f (%.1f ops/sec)\n",
            elapsed_sec, n / elapsed_sec);
    TestSuite::appendResultMessage(msg);

    // ==== find exact match key
    IntNode query, *item;
    skiplist_node *ret;

    tt.reset();
    for (i=0; i<n; ++i) {
        query.value = i*10;
        ret = skiplist_find(&list, &query.snode);
        CHK_NONNULL(ret);
        item = _get_entry(ret, IntNode, snode);
        CHK_EQ(query.value, item->value);
    }
    elapsed_sec = tt.getTimeUs() / 1000000.0;
    sprintf(msg, "find (existing key) done: %.4f (%.1f ops/sec)\n",
            elapsed_sec, n / elapsed_sec);
    TestSuite::appendResultMessage(msg);

    // ==== find smaller key
    tt.reset();

    // smaller than smallest key
    query.value = -5;
    ret = skiplist_find_smaller_or_equal(&list, &query.snode);
    CHK_NULL(ret);

    for (i=0; i<n; ++i) {
        query.value = i*10 + 5;
        ret = skiplist_find_smaller_or_equal(&list, &query.snode);
        CHK_NONNULL(ret);
        item = _get_entry(ret, IntNode, snode);
        CHK_EQ(i*10, item->value);
    }
    elapsed_sec = tt.getTimeUs() / 1000000.0;
    sprintf(msg, "find (smaller key) done: %.4f (%.1f ops/sec)\n",
            elapsed_sec, n / elapsed_sec);
    TestSuite::appendResultMessage(msg);

    // ==== find greater key
    tt.reset();

    for (i=0; i<n; ++i) {
        query.value = i*10 - 5;
        ret = skiplist_find_greater_or_equal(&list, &query.snode);
        CHK_NONNULL(ret);
        item = _get_entry(ret, IntNode, snode);
        CHK_EQ(i*10, item->value);
    }

    // greater than greatest key
    query.value = i*10;
    ret = skiplist_find_greater_or_equal(&list, &query.snode);
    CHK_NULL(ret);

    elapsed_sec = tt.getTimeUs() / 1000000.0;
    sprintf(msg, "find (greater key) done: %.4f (%.1f ops/sec)\n",
            elapsed_sec, n / elapsed_sec);
    TestSuite::appendResultMessage(msg);

    // ==== find non-existing key
    tt.reset();
    for (i=0; i<n; ++i) {
        query.value = i*10 + 1;
        ret = skiplist_find(&list, &query.snode);
        CHK_NULL(ret);
    }
    elapsed_sec = tt.getTimeUs() / 1000000.0;
    sprintf(msg, "find (non-existing key) done: %.4f (%.1f ops/sec)\n",
            elapsed_sec, n / elapsed_sec);
    TestSuite::appendResultMessage(msg);

    skiplist_free(&list);

    return 0;
}

struct thread_args : TestSuite::ThreadArgs {
    skiplist_raw* list;
    std::set<int>* stl_set;
    std::mutex *lock;
    bool use_skiplist;
    std::vector<int>* key;
    std::vector<IntNode>* arr;
    int range_begin;
    int range_end;
    double elapsed_sec;
};

struct test_args {
    int n_keys;
    int n_writers;
    int n_erasers;
    int n_readers;
    bool random_order;
    bool use_skiplist;
};

int writer_thread(void *voidargs)
{
    thread_args *args = (thread_args*)voidargs;
    TestSuite::Timer tt;

    int i;
    for (i=args->range_begin; i<=args->range_end; ++i) {
        IntNode& node = args->arr->at(i);
        if (args->use_skiplist) {
            skiplist_insert(args->list, &node.snode);
        } else {
            args->lock->lock();
            args->stl_set->insert(node.value);
            args->lock->unlock();
        }
    }
    args->elapsed_sec = tt.getTimeUs() / 1000000.0;

    return 0;
}

int eraser_thread(void *voidargs)
{
    thread_args *args = (thread_args*)voidargs;
    TestSuite::Timer tt;

    int i;
    for (i=args->range_begin; i<=args->range_end; ++i) {
        IntNode query;
        if (args->use_skiplist) {
            query.value = args->key->at(i);
            int ret = skiplist_erase(args->list, &query.snode);
            (void)ret;
        } else {
            args->lock->lock();
            args->stl_set->erase(args->key->at(i));
            args->lock->unlock();
        }
    }
    args->elapsed_sec = tt.getTimeUs() / 1000000.0;

    return 0;
}

int reader_thread(void *voidargs)
{
    thread_args *args = (thread_args*)voidargs;
    TestSuite::Timer tt;

    int i;
    for (i=args->range_begin; i<=args->range_end; ++i) {
        IntNode query;
        if (args->use_skiplist) {
            query.value = args->key->at(i);
            skiplist_node *ret = skiplist_find(args->list, &query.snode);
            (void)ret;
        } else {
            args->lock->lock();
            auto ret = args->stl_set->find(args->key->at(i));
            (void)ret;
            args->lock->unlock();
        }
    }
    args->elapsed_sec = tt.getTimeUs() / 1000000.0;

    return 0;
}

int concurrent_write_test(test_args t_args)
{
    char msg[1024];
    TestSuite::appendResultMessage("\n");

    skiplist_raw list;
    std::mutex lock;
    std::set<int> stl_set;

    skiplist_init(&list, _cmp_IntNode);

    int i, j, temp;
    int n = t_args.n_keys;
    std::vector<int> key(n);
    std::vector<IntNode> arr(n);
    // assign
    for (i=0; i<n; ++i) {
        key[i] = i;
    }

    // shuffle
    if (t_args.random_order) {
        for (i=0; i<n; ++i) {
            j = rand() % n;
            temp = key[i];
            key[i] = key[j];
            key[j] = temp;
        }
    }

    for (i=0; i<n; ++i) {
        arr[i].value = key[i];
    }

    int n_threads = t_args.n_writers;
    int n_keys_per_thread = n / n_threads;

    std::vector<TestSuite::ThreadHolder*> t_holder(n_threads);
    std::vector<thread_args> args(n_threads);

    for (i=0; i<n_threads; ++i) {
        args[i].list = &list;
        args[i].stl_set = &stl_set;
        args[i].lock = &lock;
        args[i].key = &key;
        args[i].arr = &arr;
        args[i].range_begin = i*n_keys_per_thread;
        args[i].range_end = (i+1)*n_keys_per_thread - 1;
        args[i].use_skiplist = t_args.use_skiplist;

        t_holder[i] = new TestSuite::ThreadHolder(&args[i], writer_thread, nullptr);
    }

    TestSuite::Timer tt;
    for (i=0; i<n_threads; ++i){
        t_holder[i]->join();
        delete t_holder[i];
    }
    double elapsed_sec = tt.getTimeUs() / 1000000.0;
    sprintf(msg, "insert %.4f (%d threads, %.1f ops/sec)\n",
            elapsed_sec, n_threads, n/elapsed_sec);
    TestSuite::appendResultMessage(msg);

    if (!t_args.use_skiplist) return 0;

    // integrity check (forward iteration, skiplist only)
    tt.reset();

    int count = 0;
    bool corruption = false;
    skiplist_node *cur = skiplist_begin(&list);
    while (cur) {
        IntNode *node = _get_entry(cur, IntNode, snode);
        if (node->value != count) {
            skiplist_node *missing = &arr[count].snode;
            sprintf(msg, "idx %d is missing, %lx\n", count, (uint64_t)missing);
            TestSuite::appendResultMessage(msg);

            skiplist_node *prev = skiplist_prev(&list, missing);
            skiplist_node *next = skiplist_next(&list, missing);
            IntNode *prev_node = _get_entry(prev, IntNode, snode);
            IntNode *next_node = _get_entry(next, IntNode, snode);
            sprintf(msg, "%d %d\n", prev_node->value, next_node->value);
            TestSuite::appendResultMessage(msg);

            count = node->value;
            corruption = true;
        }
        CHK_EQ(count, node->value);
        cur = skiplist_next(&list, cur);
        count++;
    }
    CHK_EQ(n, count);
    CHK_NOT(corruption);

    elapsed_sec = tt.getTimeUs() / 1000000.0;

    sprintf(msg, "iteration %.4f (%.1f ops/sec)\n",
            elapsed_sec, n/elapsed_sec);
    TestSuite::appendResultMessage(msg);

    skiplist_free(&list);

    return 0;
}

int concurrent_write_erase_test(struct test_args t_args)
{
    char msg[1024];
    TestSuite::appendResultMessage("\n");

    skiplist_raw list;
    std::mutex lock;
    std::set<int> stl_set;

    skiplist_init(&list, _cmp_IntNode);

    int i, j, temp;
    int n = t_args.n_keys;
    std::vector<int> key_add(n);
    std::vector<int> key_del(n);
    std::vector<IntNode> arr_add(n);
    std::vector<IntNode> arr_del(n);
    std::vector<IntNode*> arr_add_dbgref(n);

    // initial list state: 0, 10, 20, ...
    //  => writer threads: adding 5, 15, 25, ...
    //  => eraser threads: erasing 0, 10, 20, ...
    // final list state: 5, 15, 25, ...

    // initial insert
    for (i=0; i<n; ++i) {
        // 0, 10, 20, 30 ...
        key_del[i] = i * 10;
        arr_del[i].value = key_del[i];
        skiplist_insert(&list, &arr_del[i].snode);
    }

    // assign keys to add
    for (i=0; i<n; ++i) {
        // 5, 15, 25, 35, ...
        key_add[i] = i*10 + 5;
    }

    if (t_args.random_order) {
        // shuffle keys to add
        for (i=0; i<n; ++i) {
            j = rand() % n;
            temp = key_add[i];
            key_add[i] = key_add[j];
            key_add[j] = temp;
        }
    }
    for (i=0; i<n; ++i) {
        int ori_idx = (key_add[i] - 5) / 10;
        arr_add[i].value = key_add[i];
        arr_add_dbgref[ori_idx] = &arr_add[i];
    }

    if (t_args.random_order) {
        // also shuffle keys to delete
        for (i=0; i<n; ++i) {
            j = rand() % n;
            temp = key_del[i];
            key_del[i] = key_del[j];
            key_del[j] = temp;
        }
    }

    int n_threads_add = t_args.n_writers;
    int n_threads_del = t_args.n_erasers;

    int n_keys_per_thread_add = n / n_threads_add;
    int n_keys_per_thread_del = n / n_threads_del;

    std::vector<TestSuite::ThreadHolder*> t_holder_add(n_threads_add);
    std::vector<thread_args> args_add(n_threads_add);

    for (i=0; i<n_threads_add; ++i) {
        args_add[i].list = &list;
        args_add[i].stl_set = &stl_set;
        args_add[i].lock = &lock;
        args_add[i].key = &key_add;
        args_add[i].arr = &arr_add;
        args_add[i].range_begin = i*n_keys_per_thread_add;
        args_add[i].range_end = (i+1)*n_keys_per_thread_add - 1;
        args_add[i].use_skiplist = t_args.use_skiplist;

        t_holder_add[i] = new TestSuite::ThreadHolder
                              (&args_add[i], writer_thread, nullptr);
    }

    std::vector<TestSuite::ThreadHolder*> t_holder_del(n_threads_del);
    std::vector<thread_args> args_del(n_threads_del);

    for (i=0; i<n_threads_del; ++i) {
        args_del[i].list = &list;
        args_del[i].stl_set = &stl_set;
        args_del[i].lock = &lock;
        args_del[i].key = &key_del;
        args_del[i].arr = &arr_del;
        args_del[i].range_begin = i*n_keys_per_thread_del;
        args_del[i].range_end = (i+1)*n_keys_per_thread_del - 1;
        args_del[i].use_skiplist = t_args.use_skiplist;

        t_holder_del[i] = new TestSuite::ThreadHolder
                              (&args_del[i], eraser_thread, nullptr);
    }


    for (i=0; i<n_threads_add; ++i){
        t_holder_add[i]->join();
        delete t_holder_add[i];
    }
    for (i=0; i<n_threads_del; ++i){
        t_holder_del[i]->join();
        delete t_holder_del[i];
    }

    double max_seconds_add = 0;
    double max_seconds_del = 0;

    for (i=0; i<n_threads_add; ++i) {
        if (args_add[i].elapsed_sec > max_seconds_add) {
            max_seconds_add = args_add[i].elapsed_sec;
        }
    }

    for (i=0; i<n_threads_del; ++i) {
        if (args_del[i].elapsed_sec > max_seconds_del) {
            max_seconds_del = args_del[i].elapsed_sec;
        }
    }

    sprintf(msg, "insertion %.4f (%d threads, %.1f ops/sec)\n",
            max_seconds_add, n_threads_add, n / max_seconds_add);
    TestSuite::appendResultMessage(msg);

    sprintf(msg, "deletion %.4f (%d threads, %.1f ops/sec)\n",
            max_seconds_del, n_threads_del, n / max_seconds_del);
    TestSuite::appendResultMessage(msg);

    sprintf(msg, "mutation total %.4f (%d threads, %.1f ops/sec)\n",
            std::max(max_seconds_add, max_seconds_del),
            n_threads_add + n_threads_del,
            (n*2) / std::max(max_seconds_add, max_seconds_del));
    TestSuite::appendResultMessage(msg);

    if (!t_args.use_skiplist) {
        return 0;
    }

    // integrity check (forward iteration, skiplist only)
    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::chrono::duration<double> elapsed_seconds;
    start = std::chrono::system_clock::now();

    int count = 0;
    bool corruption = false;
    skiplist_node *cur = skiplist_begin(&list);
    std::vector<skiplist_node*> dbg_node(n);
    std::vector<IntNode*> dbg_int(n);

    while (cur) {
        IntNode *node = _get_entry(cur, IntNode, snode);
        dbg_node[count] = cur;
        dbg_int[count] = node;
        // 5, 15, 25, 35 ...
        int idx = count * 10 + 5;
        if (node->value != idx) {
            skiplist_node *missing = &arr_add_dbgref[count]->snode;
            sprintf(msg, "count %d, idx %d is missing %lx\n",
                    count, idx, (uint64_t)missing);
            TestSuite::appendResultMessage(msg);

            skiplist_node *prev = skiplist_prev(&list, missing);
            skiplist_node *next = skiplist_next(&list, missing);
            IntNode *prev_node = _get_entry(prev, IntNode, snode);
            IntNode *next_node = _get_entry(next, IntNode, snode);
            sprintf(msg, "%d %d\n", prev_node->value, next_node->value);
            TestSuite::appendResultMessage(msg);

            corruption = true;
        }
        CHK_EQ(idx, node->value);
        cur = skiplist_next(&list, cur);
        count++;
    }
    CHK_EQ(n, count);
    CHK_NOT(corruption);

    end = std::chrono::system_clock::now();
    elapsed_seconds = end-start;

    sprintf(msg, "iteration %.4f (%.1f ops/sec)\n",
            elapsed_seconds.count(), n/elapsed_seconds.count());
    TestSuite::appendResultMessage(msg);

    skiplist_free(&list);

    return 0;
}

int concurrent_write_read_test(struct test_args t_args)
{
    char msg[1024];
    TestSuite::appendResultMessage("\n");

    skiplist_raw list;
    std::mutex lock;
    std::set<int> stl_set;

    skiplist_init(&list, _cmp_IntNode);

    int i, j, temp;
    int n = t_args.n_keys;
    std::vector<int> key_add(n);
    std::vector<int> key_read(n);
    std::vector<IntNode> arr_add(n);
    std::vector<IntNode> arr_find(n);
    std::vector<IntNode*> arr_add_dbgref(n);

    // initial list state: 0, 10, 20, ...
    //  => writer threads: adding 5, 15, 25, ...
    //  => reader threads: reading 0, 10, 20, ...
    // final list state: 0, 5, 10, 15, 20, ...

    // initial insert
    for (i=0; i<n; ++i) {
        // 0, 10, 20, 30 ...
        key_read[i] = i * 10;
        arr_find[i].value = key_read[i];
        skiplist_insert(&list, &arr_find[i].snode);
    }

    // assign keys to add
    for (i=0; i<n; ++i) {
        // 5, 15, 25, 35, ...
        key_add[i] = i*10 + 5;
    }

    if (t_args.random_order) {
        // shuffle keys to add
        for (i=0; i<n; ++i) {
            j = rand() % n;
            temp = key_add[i];
            key_add[i] = key_add[j];
            key_add[j] = temp;
        }
    }
    for (i=0; i<n; ++i) {
        int ori_idx = (key_add[i] - 5) / 10;
        arr_add[i].value = key_add[i];
        arr_add_dbgref[ori_idx] = &arr_add[i];
    }

    if (t_args.random_order) {
        // also shuffle keys to find
        for (i=0; i<n; ++i) {
            j = rand() % n;
            temp = key_read[i];
            key_read[i] = key_read[j];
            key_read[j] = temp;
        }
    }

    int n_threads_add = t_args.n_writers;
    int n_threads_find = t_args.n_readers;

    std::vector<TestSuite::ThreadHolder*> t_holder_add(n_threads_add);
    std::vector<TestSuite::ThreadHolder*> t_holder_find(n_threads_find);
    std::vector<thread_args> args_add(n_threads_add);
    std::vector<thread_args> args_find(n_threads_find);

    if (n_threads_add) {
        int n_keys_per_thread_add = n / n_threads_add;

        for (i=0; i<n_threads_add; ++i) {
            args_add[i].list = &list;
            args_add[i].stl_set = &stl_set;
            args_add[i].lock = &lock;
            args_add[i].key = &key_add;
            args_add[i].arr = &arr_add;
            args_add[i].range_begin = i*n_keys_per_thread_add;
            args_add[i].range_end = (i+1)*n_keys_per_thread_add - 1;
            args_add[i].use_skiplist = t_args.use_skiplist;

            t_holder_add[i] = new TestSuite::ThreadHolder
                              (&args_add[i], writer_thread, nullptr);
        }
    }

    if (n_threads_find) {
        int n_keys_per_thread_find = n / n_threads_find;

        for (i=0; i<n_threads_find; ++i) {
            args_find[i].list = &list;
            args_find[i].stl_set = &stl_set;
            args_find[i].lock = &lock;
            args_find[i].key = &key_read;
            args_find[i].arr = &arr_find;
            args_find[i].range_begin = i*n_keys_per_thread_find;
            args_find[i].range_end = (i+1)*n_keys_per_thread_find - 1;
            args_find[i].use_skiplist = t_args.use_skiplist;

            t_holder_find[i] = new TestSuite::ThreadHolder
                               (&args_find[i], reader_thread, nullptr);
        }
    }

    for (i=0; i<n_threads_add; ++i){
        t_holder_add[i]->join();
        delete t_holder_add[i];
    }
    for (i=0; i<n_threads_find; ++i){
        t_holder_find[i]->join();
        delete t_holder_find[i];
    }

    if (n_threads_add) {
        double max_seconds_add = 0;
        for (i=0; i<n_threads_add; ++i) {
            if (args_add[i].elapsed_sec > max_seconds_add) {
                max_seconds_add = args_add[i].elapsed_sec;
            }
        }
        sprintf(msg, "insertion %.4f (%d threads, %.1f ops/sec)\n",
                max_seconds_add, n_threads_add, n / max_seconds_add);
        TestSuite::appendResultMessage(msg);
    }

    if (n_threads_find) {
        double max_seconds_find = 0;
        for (i=0; i<n_threads_find; ++i) {
            if (args_find[i].elapsed_sec > max_seconds_find) {
                max_seconds_find = args_find[i].elapsed_sec;
            }
        }
        sprintf(msg, "retrieval %.4f (%d threads, %.1f ops/sec)\n",
                max_seconds_find, n_threads_find, n / max_seconds_find);
        TestSuite::appendResultMessage(msg);
    }

    skiplist_free(&list);

    return 0;
}

int main(int argc, char** argv) {
    TestSuite ts(argc, argv);
    srand(0xabcd);

    struct test_args args;
    args.n_keys = 40000;
    args.random_order = true;
    args.use_skiplist = true;

    //ts.options.printTestMessage = true;
    ts.doTest("basic insert and erase", basic_insert_and_erase);
    ts.doTest("find test", find_test);

    args.n_writers = 8;
    ts.doTest("concurrent write test", concurrent_write_test, args);

    args.n_writers = 4;
    args.n_erasers = 4;
    ts.doTest("concurrent write erase test", concurrent_write_erase_test, args);

    args.n_writers = 1;
    args.n_readers = 7;
    ts.doTest("concurrent write read test", concurrent_write_read_test, args);

    return 0;
}



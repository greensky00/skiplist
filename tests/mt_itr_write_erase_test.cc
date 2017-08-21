#include <stdio.h>
#include <unistd.h>

#include <chrono>
#include <thread>
#include <vector>

#include "skiplist.h"
#include "test_common.h"

struct TestNode {
    TestNode() : value(0) {
        skiplist_init_node(&snode);
    }
    ~TestNode() {
        skiplist_free_node(&snode);
    }

    static int cmp(skiplist_node* a, skiplist_node* b, void* aux) {
        TestNode *aa, *bb;
        aa = _get_entry(a, TestNode, snode);
        bb = _get_entry(b, TestNode, snode);
        if (aa->value < bb->value) return -1;
        if (aa->value > bb->value) return 1;
        return 0;
    }

    skiplist_node snode;
    int value;
};

struct ThreadArgs {
    skiplist_raw* slist;
    int max_num;
    int duration_ms;
    int ret;
};

int _itr_thread(ThreadArgs* args) {
    std::chrono::time_point<std::chrono::system_clock> start, cur;
    std::chrono::duration<double> elapsed;

    start = std::chrono::system_clock::now();

    do {
        int num_walks = 10;
        int count = 0;
        int r = rand() % args->max_num;

        TestNode query;
        query.value = r;
        skiplist_node* cursor = skiplist_find(args->slist, &query.snode);
        while (cursor) {
            TestNode* node = _get_entry(cursor, TestNode, snode);
            cursor = skiplist_next(args->slist, cursor);
            usleep(10);
            (void)node;
            skiplist_release_node(&node->snode);
            if (++count > num_walks) break;
        }
        if (cursor) skiplist_release_node(cursor);

        cur = std::chrono::system_clock::now();
        elapsed = cur - start;
    } while (args->duration_ms > elapsed.count() * 1000);

    return 0;
}

void itr_thread(ThreadArgs* args) {
    args->ret = _itr_thread(args);
}

void writer_thread(ThreadArgs* args) {
    std::chrono::time_point<std::chrono::system_clock> start, cur;
    std::chrono::duration<double> elapsed;

    start = std::chrono::system_clock::now();

    do {
        int r;
        TestNode* node;
        TestNode query;
        skiplist_node* cursor;

        r = rand() % args->max_num;
        query.value = r;
        cursor = skiplist_find(args->slist, &query.snode);
        if (!cursor) {
            node = new TestNode();
            skiplist_init_node(&node->snode);
            node->value = r;
            skiplist_insert(args->slist, &node->snode);
        } else {
            skiplist_release_node(cursor);
        }

        r = rand() % args->max_num;
        query.value = r;
        cursor = skiplist_find(args->slist, &query.snode);
        if (cursor) {
            node = _get_entry(cursor, TestNode, snode);
            skiplist_erase_node(args->slist, &node->snode);
            if (node->snode.being_modified ||
                !node->snode.removed)
                printf("%d %d\n", (int)node->snode.being_modified,
                    (int)node->snode.removed);
            skiplist_release_node(&node->snode);
            while (!skiplist_is_safe_to_free(&node->snode)) {
                usleep(10);
            }
            delete node;
        }

        cur = std::chrono::system_clock::now();
        elapsed = cur - start;
    } while (args->duration_ms > elapsed.count() * 1000);
}

int itr_write_erase() {
    ThreadArgs args;
    std::thread iterator;
    std::thread writer;
    int num = 30000;

    skiplist_raw slist;
    skiplist_init(&slist, TestNode::cmp);
    std::vector<TestNode*> node(num);

    for (int ii=0; ii<num; ii+=1) {
        node[ii] = new TestNode();
        node[ii]->value = ii;
        skiplist_insert(&slist, &node[ii]->snode);
    }
    for (int ii=0; ii<num; ii+=1) {
        if (node[ii]->snode.ref_count)
            printf("%d %d\n", ii, node[ii]->snode.ref_count);
    }

    for (int ii=0; ii<num; ii+=2) {
        skiplist_erase_node(&slist, &node[ii]->snode);
    }
    for (int ii=0; ii<num; ii+=1) {
        if (node[ii]->snode.ref_count)
            printf("%d %d\n", ii, node[ii]->snode.ref_count);
    }

    args.slist = &slist;
    args.duration_ms = 1000;
    args.max_num = num;
    iterator = std::thread(itr_thread, &args);
    writer = std::thread(writer_thread, &args);

    iterator.join();
    writer.join();

    for (int ii=0; ii<num; ii+=1) {
        if (node[ii]->snode.ref_count)
            printf("%d %d\n", ii, node[ii]->snode.ref_count);
    }

    skiplist_node* cursor = skiplist_begin(&slist);
    while(cursor) {
        TestNode* cur_node = _get_entry(cursor, TestNode, snode);
        if (cur_node->snode.ref_count != 1)
            printf("%d %d\n", (int)cur_node->value, (int)cur_node->snode.ref_count);
        cursor = skiplist_next(&slist, cursor);
        skiplist_release_node(&cur_node->snode);
    }
    if (cursor) skiplist_release_node(cursor);

    skiplist_free(&slist);
    return 0;
}

int itr_erase_deterministic() {
    int num = 10;
    skiplist_raw slist;
    skiplist_init(&slist, TestNode::cmp);

    TestNode* node[num];
    for (int ii=0; ii<num; ++ii) {
        node[ii] = new TestNode();
        node[ii]->value = ii;
        skiplist_insert(&slist, &node[ii]->snode);
    }

    skiplist_node* cursor = skiplist_begin(&slist);
    while (cursor) {
        TestNode* cur_node = _get_entry(cursor, TestNode, snode);
        if (cur_node->value == 2) {
            skiplist_erase_node(&slist, &cur_node->snode);
            CHK_NOT(skiplist_is_safe_to_free(&cur_node->snode));
        }
        cursor = skiplist_next(&slist, cursor);
        skiplist_release_node(&cur_node->snode);
        if (cur_node->value == 2) {
            CHK_OK(skiplist_is_safe_to_free(&cur_node->snode));
            delete cur_node;
        }
    }
    if (cursor) skiplist_release_node(cursor);

    int count = 0;
    cursor = skiplist_begin(&slist);
    while (cursor) {
        TestNode* cur_node = _get_entry(cursor, TestNode, snode);
        cursor = skiplist_next(&slist, cursor);
        skiplist_release_node(&cur_node->snode);
        count++;
    }
    if (cursor) skiplist_release_node(cursor);
    CHK_EQ(num-1, count);

    skiplist_free(&slist);
    return 0;
}

int main(int argc, char** argv) {
    TestSuite tt(argc, argv);

    tt.doTest("iterator write erase test", itr_write_erase);
    tt.doTest("iterator write erase deterministic test", itr_erase_deterministic);

    return 0;
}


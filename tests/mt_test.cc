#include "skiplist.h"

#include "test_common.h"

#include <chrono>
#include <thread>
#include <vector>

#include <stdio.h>
#include <unistd.h>

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
    TestSuite::Timer timer(args->duration_ms);
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
    } while (!timer.timeover());

    return 0;
}

void itr_thread(ThreadArgs* args) {
    args->ret = _itr_thread(args);
}

int _writer_thread(ThreadArgs* args) {
    TestSuite::Timer timer(args->duration_ms);
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
            skiplist_wait_for_free(&node->snode);
            delete node;
        }
    } while (!timer.timeover());

    uint64_t c_check = 0;
    skiplist_node* cursor = skiplist_begin(args->slist);
    while (cursor) {
        skiplist_node* temp_node = cursor;
        cursor = skiplist_next(args->slist, cursor);
        skiplist_release_node(temp_node);
        c_check++;
    }
    if (cursor) skiplist_release_node(cursor);

    CHK_EQ(c_check, skiplist_get_size(args->slist));
    return 0;
}

void writer_thread(ThreadArgs* args) {
    args->ret = _writer_thread(args);
}

int itr_write_erase() {
    std::thread iterator;
    std::thread writer;
    int num = 30000;

    skiplist_raw slist;
    skiplist_init(&slist, TestNode::cmp);
    TestNode* node[num];

    for (int ii=0; ii<num; ii+=1) {
        node[ii] = new TestNode();
        node[ii]->value = ii;
        skiplist_insert(&slist, &node[ii]->snode);
    }
    CHK_EQ(num, (int)skiplist_get_size(&slist));

    for (int ii=0; ii<num; ii+=1) {
        CHK_EQ(0, node[ii]->snode.ref_count);
    }

    for (int ii=0; ii<num; ii+=2) {
        skiplist_erase_node(&slist, &node[ii]->snode);
        delete node[ii];
    }
    for (int ii=1; ii<num; ii+=2) {
        CHK_EQ(0, node[ii]->snode.ref_count);
    }

    ThreadArgs args_itr;
    ThreadArgs args_writer;
    args_itr.slist = &slist;
    args_itr.duration_ms = 1000;
    args_itr.max_num = num;
    args_itr.ret = 0;
    args_writer = args_itr;
    iterator = std::thread(itr_thread, &args_itr);
    writer = std::thread(writer_thread, &args_writer);

    iterator.join();
    writer.join();

    CHK_EQ(0, args_itr.ret);
    CHK_EQ(0, args_writer.ret);

    skiplist_node* cursor = skiplist_begin(&slist);
    while(cursor) {
        TestNode* cur_node = _get_entry(cursor, TestNode, snode);
        if (cur_node->snode.ref_count != 1)
            printf("%d %d\n", (int)cur_node->value, (int)cur_node->snode.ref_count);
        CHK_EQ(1, cur_node->snode.ref_count);
        cursor = skiplist_next(&slist, cursor);
        skiplist_release_node(&cur_node->snode);
        delete cur_node;
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
        delete cur_node;
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


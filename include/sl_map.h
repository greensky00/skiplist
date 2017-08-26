/**
 * Copyright (C) 2017-present Jung-Sang Ahn <jungsang.ahn@gmail.com>
 * All rights reserved.
 *
 * https://github.com/greensky00
 *
 * Skiplist map container
 * Version: 0.1.0
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

#include <utility> // std::pair

#include "skiplist.h"

template<typename K, typename V>
class sl_map {
private:
    using T = std::pair<K, V>;

public:
    class iterator {
        friend class sl_map;
    public:
        iterator() : slist(nullptr), cursor(nullptr) {}
        ~iterator() {
            if (cursor) skiplist_release_node(cursor);
        }

        void operator=(const iterator& src) {
            // This reference counting is similar to that of shared_ptr.
            skiplist_node* tmp = cursor;
            if (src.cursor)
                skiplist_grab_node(src.cursor);
            cursor = src.cursor;
            if (tmp)
                skiplist_release_node(tmp);
        }

        bool operator==(const iterator& src) const { return (cursor == src.cursor); }
        bool operator!=(const iterator& src) const { return !operator==(src); }

        T* operator->() const {
            Node* node = _get_entry(cursor, Node, snode);
            return &node->kv;
        }
        T& operator*() const {
            Node* node = _get_entry(cursor, Node, snode);
            return node->kv;
        }

        // ++A
        iterator& operator++() {
            if (!slist || !cursor) {
                cursor = nullptr;
                return *this;
            }
            skiplist_node* next = skiplist_next(slist, cursor);
            skiplist_release_node(cursor);
            cursor = next;
            return *this;
        }
        // A++
        iterator& operator++(int) { return operator++(); }
        // --A
        iterator& operator--() {
            if (!slist || !cursor) {
                cursor = nullptr;
                return *this;
            }
            skiplist_node* prev = skiplist_prev(slist, cursor);
            skiplist_release_node(cursor);
            cursor = prev;
            return *this;
        }
        // A--
        iterator operator--(int) { return operator--(); }

    private:
        iterator(skiplist_raw* _slist,
                 skiplist_node* _cursor)
            : slist(_slist), cursor(_cursor) {}

        skiplist_raw* slist;
        skiplist_node* cursor;
    };

    using reverse_iterator = iterator;

    sl_map() {
        skiplist_init(&slist, Node::cmp);
    }
    ~sl_map() {
        skiplist_node* cursor = skiplist_begin(&slist);
        while (cursor) {
            Node* node = _get_entry(cursor, Node, snode);
            cursor = skiplist_next(&slist, cursor);
            skiplist_erase_node(&slist, &node->snode);
            skiplist_release_node(&node->snode);
            delete node;
        }
        if (cursor) skiplist_release_node(cursor);
        skiplist_free(&slist);
    }

    bool empty() {
        skiplist_node* cursor = skiplist_begin(&slist);
        if (cursor) {
            skiplist_release_node(cursor);
            return false;
        }
        return true;
    }

    size_t size() { return skiplist_get_size(&slist); }

    void insert(std::pair<K, V> kv) {
        Node* node = new Node();
        node->kv = kv;
        skiplist_insert(&slist, &node->snode);
    }

    iterator find(K key) {
        Node query;
        query.kv.first = key;
        skiplist_node* cursor = skiplist_find(&slist, &query.snode);
        return iterator(&slist, cursor);
    }

    iterator erase(iterator& position) {
        skiplist_node* cursor = position.cursor;
        skiplist_node* next = skiplist_next(&slist, cursor);

        skiplist_erase_node(&slist, cursor);
        skiplist_release_node(cursor);
        skiplist_wait_for_free(cursor);
        Node* node = _get_entry(cursor, Node, snode);
        delete node;

        position.cursor = nullptr;
        return iterator(&slist, next);
    }

    size_t erase(const K& key) {
        size_t count = 0;
        Node query;
        query.kv.first = key;
        skiplist_node* cursor = skiplist_find(&slist, &query.snode);
        while (cursor) {
            Node* node = _get_entry(cursor, Node, snode);
            if (node->kv.first != key) break;

            cursor = skiplist_next(&slist, cursor);

            skiplist_erase_node(&slist, &node->snode);
            skiplist_release_node(&node->snode);
            skiplist_wait_for_free(&node->snode);
            delete node;
        }
        if (cursor) skiplist_release_node(cursor);
        return count;
    }

    iterator begin() {
        skiplist_node* cursor = skiplist_begin(&slist);
        return iterator(&slist, cursor);
    }
    iterator end() { return iterator(); }

    reverse_iterator rbegin() {
        skiplist_node* cursor = skiplist_end(&slist);
        return reverse_iterator(&slist, cursor);
    }
    reverse_iterator rend() { return reverse_iterator(); }

private:
    struct Node {
        Node() {
            skiplist_init_node(&snode);
        }
        ~Node() {
            skiplist_free_node(&snode);
        }
        static int cmp(skiplist_node* a, skiplist_node* b, void* aux) {
            Node *aa, *bb;
            aa = _get_entry(a, Node, snode);
            bb = _get_entry(b, Node, snode);
            if (aa->kv.first < bb->kv.first) return -1;
            if (aa->kv.first > bb->kv.first) return 1;
            return 0;
        }

        skiplist_node snode;
        T kv;
    };

    skiplist_raw slist;
};


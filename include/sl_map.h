/**
 * Copyright (C) 2017-present Jung-Sang Ahn <jungsang.ahn@gmail.com>
 * All rights reserved.
 *
 * https://github.com/greensky00
 *
 * Skiplist map container
 * Version: 0.2.0
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

#include "skiplist.h"

#include <atomic>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

template<typename K, typename V>
struct map_node {
    map_node() {
        skiplist_init_node(&snode);
    }
    ~map_node() {
        skiplist_free_node(&snode);
    }
    static int cmp(skiplist_node* a, skiplist_node* b, void* aux) {
        map_node *aa, *bb;
        aa = _get_entry(a, map_node, snode);
        bb = _get_entry(b, map_node, snode);
        if (aa->kv.first < bb->kv.first) return -1;
        if (aa->kv.first > bb->kv.first) return 1;
        return 0;
    }

    skiplist_node snode;
    std::pair<K, V> kv;
};

template<typename K, typename V> class sl_map;
template<typename K, typename V> class sl_map_gc;

template<typename K, typename V>
class map_iterator {
    friend class sl_map<K, V>;
    friend class sl_map_gc<K, V>;

private:
    using T = std::pair<K, V>;
    using Node = map_node<K, V>;

public:
    map_iterator() : slist(nullptr), cursor(nullptr) {}

    map_iterator(map_iterator&& src)
        : slist(src.slist), cursor(src.cursor)
    {
        // Mimic perfect forwarding.
        src.slist = nullptr;
        src.cursor = nullptr;
    }

    ~map_iterator() {
        if (cursor) skiplist_release_node(cursor);
    }

    void operator=(const map_iterator& src) {
        // This reference counting is similar to that of shared_ptr.
        skiplist_node* tmp = cursor;
        if (src.cursor)
            skiplist_grab_node(src.cursor);
        cursor = src.cursor;
        if (tmp)
            skiplist_release_node(tmp);
    }

    bool operator==(const map_iterator& src) const { return (cursor == src.cursor); }
    bool operator!=(const map_iterator& src) const { return !operator==(src); }

    T* operator->() const {
        Node* node = _get_entry(cursor, Node, snode);
        return &node->kv;
    }
    T& operator*() const {
        Node* node = _get_entry(cursor, Node, snode);
        return node->kv;
    }

    // ++A
    map_iterator& operator++() {
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
    map_iterator& operator++(int) { return operator++(); }
    // --A
    map_iterator& operator--() {
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
    map_iterator operator--(int) { return operator--(); }

private:
    map_iterator(skiplist_raw* _slist,
                 skiplist_node* _cursor)
        : slist(_slist), cursor(_cursor) {}

    skiplist_raw* slist;
    skiplist_node* cursor;
};



template<typename K, typename V>
class sl_map {
private:
    using T = std::pair<K, V>;
    using Node = map_node<K, V>;

public:
    using iterator = map_iterator<K, V>;
    using reverse_iterator = map_iterator<K, V>;

    sl_map() {
        skiplist_init(&slist, Node::cmp);
    }

    virtual
    ~sl_map() {
        skiplist_node* cursor = skiplist_begin(&slist);
        while (cursor) {
            Node* node = _get_entry(cursor, Node, snode);
            cursor = skiplist_next(&slist, cursor);
            // Don't need to care about release.
            delete node;
        }
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

    std::pair<iterator, bool> insert(const std::pair<K, V>& kv) {
        do {
            Node* node = new Node();
            node->kv = kv;

            int rc = skiplist_insert_nodup(&slist, &node->snode);
            if (rc == 0) {
                skiplist_grab_node(&node->snode);
                return std::pair<iterator, bool>
                       ( iterator(&slist, &node->snode), true );
            }
            delete node;

            Node query;
            query.kv.first = kv.first;
            skiplist_node* cursor = skiplist_find(&slist, &query.snode);
            if (cursor) {
                return std::pair<iterator, bool>
                       ( iterator(&slist, cursor), false );
            }
        } while (true);

        // NOTE: Should not reach here.
        return std::pair<iterator, bool>(iterator(), false);
    }

    iterator find(K key) {
        Node query;
        query.kv.first = key;
        skiplist_node* cursor = skiplist_find(&slist, &query.snode);
        return iterator(&slist, cursor);
    }

    virtual
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

    virtual
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

protected:
    skiplist_raw slist;
};


template<typename K, typename V>
class sl_map_gc : public sl_map<K, V> {
private:
    using T = std::pair<K, V>;
    using Node = map_node<K, V>;

public:
    using iterator = map_iterator<K, V>;
    using reverse_iterator = map_iterator<K, V>;

    sl_map_gc()
        : sl_map<K, V>()
        , gcVector( std::max( (size_t)4,
                              (size_t)std::thread::hardware_concurrency() ) )
    {
        for (auto& entry: gcVector) {
            entry = new std::atomic<Node*>(nullptr);
        }
    }

    ~sl_map_gc() {
        execGc();
        for (std::atomic<Node*>*& a_node: gcVector) {
            Node* node = a_node->load();
            delete node;
            delete a_node;
        }
    }

    iterator erase(iterator& position) {
        skiplist_node* cursor = position.cursor;
        skiplist_node* next = skiplist_next(&this->slist, cursor);

        skiplist_erase_node(&this->slist, cursor);

        Node* node = _get_entry(cursor, Node, snode);
        gcPush(node);
        skiplist_release_node(cursor);
        execGc();

        position.cursor = nullptr;
        return iterator(&this->slist, next);
    }

    size_t erase(const K& key) {
        size_t count = 0;
        Node query;
        query.kv.first = key;
        skiplist_node* cursor = skiplist_find(&this->slist, &query.snode);
        while (cursor) {
            Node* node = _get_entry(cursor, Node, snode);
            if (node->kv.first != key) break;

            cursor = skiplist_next(&this->slist, cursor);

            skiplist_erase_node(&this->slist, &node->snode);
            gcPush(node);
            skiplist_release_node(&node->snode);
        }
        if (cursor) skiplist_release_node(cursor);

        execGc();
        return count;
    }

private:
    void gcPush(Node* node) {
        size_t v_len = gcVector.size();

        do {
            size_t rr = std::rand() % v_len;
            for (size_t ii = rr; ii < rr + v_len; ++ii) {
                std::atomic<Node*>& a_node = *gcVector[ii % v_len];

                Node* exp = nullptr;
                if ( a_node.compare_exchange_strong
                            ( exp, node, std::memory_order_relaxed ) ) {
                    return;
                }
            }

            std::this_thread::yield();
            execGc();
        } while (true);
    }

    void execGc() {
        std::unique_lock<std::mutex> l(gcVectorLock, std::try_to_lock);
        if (!l.owns_lock()) return;

        size_t v_len = gcVector.size();
        for (size_t ii = 0; ii < v_len; ++ii) {
            std::atomic<Node*>& a_node = *gcVector[ii];
            Node* node = a_node.load();
            if (!node) continue;

            if (skiplist_is_safe_to_free(&node->snode)) {
                Node* exp = node;
                Node* val = nullptr;
                a_node.compare_exchange_strong
                       ( exp, val, std::memory_order_relaxed );

                delete node;
            }
        }
    }

    std::mutex gcVectorLock;
    std::vector< std::atomic<Node*>* > gcVector;
};



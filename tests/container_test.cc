#include "sl_map.h"
#include "sl_set.h"

#include "test_common.h"

#include <chrono>
#include <thread>
#include <vector>

#include <stdio.h>
#include <unistd.h>

int _map_basic(sl_map<int, int>& sl) {
    for (int i=0; i<10; ++i) {
        auto itr = sl.insert( std::make_pair(i, i*10) );
        CHK_TRUE(itr.second);
        CHK_EQ(i, itr.first->first);
        CHK_EQ(i*10, itr.first->second);
    }

    // Duplicate insert should not be allowed.
    for (int i=0; i<10; ++i) {
        auto itr = sl.insert( std::make_pair(i, i*20) );
        CHK_FALSE(itr.second);
        CHK_EQ(i, itr.first->first);
        CHK_EQ(i*10, itr.first->second);
    }

    for (int i=0; i<10; ++i) {
        auto entry = sl.find(i);
        if (entry != sl.end()) {
            CHK_EQ(i, entry->first);
            CHK_EQ(i*10, entry->second);
        }
    }

    auto ii = sl.find(5);
    sl.erase(ii);

    int count = 0;
    for (auto& entry: sl) {
        CHK_EQ(count, entry.first);
        CHK_EQ(count*10, entry.second);
        if (count == 4) count += 2;
        else count++;
    }

    ii = sl.begin();
    while (ii != sl.end()) {
        if (ii->first % 2 == 0) {
            ii = sl.erase(ii);
        } else {
            ii++;
        }
    }

    count = 1;
    for (auto& entry: sl) {
        CHK_EQ(count, entry.first);
        CHK_EQ(count*10, entry.second);
        if (count == 3) count += 4;
        else count += 2;
    }
    CHK_EQ(4, (int)sl.size());

    return 0;
}

int map_basic_wait() {
    sl_map<int, int> sl_wait;
    return _map_basic(sl_wait);
}

int map_basic_gc() {
    sl_map_gc<int, int> sl_gc;
    return _map_basic(sl_gc);
}

int _set_basic(sl_set<int>& sl) {
    for (int i=0; i<10; ++i) {
        auto itr = sl.insert(i);
        CHK_TRUE(itr.second);
        CHK_EQ(i, *itr.first);
    }

    // Duplicate insert should not be allowed.
    for (int i=0; i<10; ++i) {
        auto itr = sl.insert(i);
        CHK_FALSE(itr.second);
        CHK_EQ(i, *itr.first);
    }

    for (int i=0; i<10; ++i) {
        auto entry = sl.find(i);
        if (entry != sl.end()) {
            CHK_EQ(i, *entry);
        }
    }

    auto ii = sl.find(5);
    sl.erase(ii);

    int count = 0;
    for (auto& entry: sl) {
        int val = entry;
        CHK_EQ(count, val);
        if (count == 4) count += 2;
        else count++;
    }

    ii = sl.begin();
    while (ii != sl.end()) {
        int val = *ii;
        if (val % 2 == 0) {
            ii = sl.erase(ii);
        } else {
            ii++;
        }
    }

    count = 1;
    for (auto& entry: sl) {
        int val = entry;
        CHK_EQ(count, val);
        if (count == 3) count += 4;
        else count += 2;
    }
    CHK_EQ(4, (int)sl.size());

    return 0;
}

int set_basic_wait() {
    sl_set<int> sl_wait;
    return _set_basic(sl_wait);
}

int set_basic_gc() {
    sl_set_gc<int> sl_gc;
    return _set_basic(sl_gc);
}

int map_self_refer_test() {
    sl_map_gc<int, int> sl;
    for (int i=0; i<10; ++i) {
        auto itr = sl.insert( std::make_pair(i, i*10) );
        CHK_TRUE(itr.second);
        CHK_EQ(i, itr.first->first);
        CHK_EQ(i*10, itr.first->second);
    }

    for(;;) {
        auto entry = sl.begin();
        if (entry == sl.end()) break;
        sl.erase(entry->first);
    }
    return 0;
}

int set_self_refer_test() {
    sl_set_gc<int> sl;
    for (int i=0; i<10; ++i) {
        auto itr = sl.insert(i);
        CHK_TRUE(itr.second);
        CHK_EQ(i, *itr.first);
    }

    for(;;) {
        auto entry = sl.begin();
        if (entry == sl.end()) break;
        sl.erase(*entry);
    }
    return 0;
}

int main(int argc, char** argv) {
    TestSuite tt(argc, argv);

    tt.doTest("container map test (busy wait)", map_basic_wait);
    tt.doTest("container map test (lazy gc)", map_basic_gc);
    tt.doTest("container map self refer test", map_self_refer_test);
    tt.doTest("container set test (busy wait)", set_basic_wait);
    tt.doTest("container set test (lazy gc)", set_basic_gc);
    tt.doTest("container set self refer test", set_self_refer_test);

    return 0;
}


#include <stdio.h>
#include <unistd.h>

#include <chrono>
#include <thread>
#include <vector>

#include "sl_map.h"
#include "sl_set.h"

#include "test_common.h"

int map_basic() {
    sl_map<int, int> sl;

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

int set_basic() {
    sl_set<int> sl;

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

int main(int argc, char** argv) {
    TestSuite tt(argc, argv);

    tt.doTest("container map test", map_basic);
    tt.doTest("container set test", set_basic);

    return 0;
}


#include <stdio.h>

#include "sl_set.h"

int main() {
    sl_set<int> slist;

    //   << Insertion >>
    // Insert 3 integers: 0, 1, and 2.
    for (int i=0; i<3; ++i) {
        slist.insert(i);
    }

    //   << Point lookup >>
    for (int i=0; i<3; ++i) {
        auto itr = slist.find(i);
        if (itr == slist.end()) continue; // Not found.
        printf("[point lookup] %d\n", *itr);

        // Note: while `itr` is alive and holding a node in skiplist,
        //       other thread cannot erase and free the node.
        //       Same as `shared_ptr`, `itr` will automatically release
        //       the node when it is not referred anymore.
        //       But if you want to release the node before that,
        //       you can do it as follows:
        // itr = slist.end();
    }

    //   << Erase >>
    // Erase 1.
    slist.erase(1);

    //   << Iteration >>
    for (auto& entry: slist) {
        printf("[iteration] %d\n", entry);
    }

    return 0;
}

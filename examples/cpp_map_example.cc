#include "sl_map.h"

#include <stdio.h>

int main() {
    // sl_map: Busy-waiting implementation.
    //         erase() API may be blocked by concurrent
    //         operations dealing with iterator on the
    //         same key.
    //
    // sl_map_gc: Lazy reclaiming implementation.
    //            erase() API will not be blocked by
    //            any concurrent operations, but may
    //            consume more memory.

    // sl_map<int, int> slist;
    sl_map_gc<int, int> slist;

    //   << Insertion >>
    // Insert 3 KV pairs: {0, 0}, {1, 10}, {2, 20}.
    for (int i=0; i<3; ++i) {
        slist.insert(std::make_pair(i, i*10));
    }

    //   << Point lookup >>
    for (int i=0; i<3; ++i) {
        auto itr = slist.find(i);
        if (itr == slist.end()) continue; // Not found.
        printf("[point lookup] key: %d, value: %d\n", itr->first, itr->second);

        // Note: In `sl_map`, while `itr` is alive and holding a node
        //       in skiplist, other thread cannot erase and free the node.
        //       Same as `shared_ptr`, `itr` will automatically release
        //       the node when it is not referred anymore.
        //       But if you want to release the node before that,
        //       you can do it as follows:
        // itr = slist.end();
    }

    //   << Erase >>
    // Erase the KV pair for key 1: {1, 10}.
    slist.erase(1);

    //   << Iteration >>
    for (auto& entry: slist) {
        printf("[iteration] key: %d, value: %d\n", entry.first, entry.second);
    }

    return 0;
}


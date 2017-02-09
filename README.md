Skiplist
--------
A generic Skiplist container C implementation, lock free for both multiple readers and writers. It can be used as a set or a map, containing any type of data.

It basically uses STL atomic variables with C++ compiler, but they can be switched to built-in GCC atomic operations when we compile it with pure C compiler.



Author
------
Jung-Sang Ahn <jungsang.ahn@gmail.com>


Build
-----
```sh
$ make
```


How to use
----------
(refer to ```tests/skiplist_test.cc```)

Below example describes how to use Skiplist as an ordered map of integer pairs.

We define a node for an integer pair, and a comparison function of given two nodes:
```C
#include "skiplist.h"

struct kv_node{
    skiplist_node snode;
    // put your data here
    int key;
    int value;
};

int cmp_func(skiplist_node *a, skiplist_node *b, void *aux)
{
    struct kv_node *aa, *bb;
    aa = _get_entry(a, struct kv_node, snode);
    bb = _get_entry(b, struct kv_node, snode);

    if (aa->key < bb->key)
        return -1;
    else if (aa->key > bb->key)
        return 1;
    else
        return 0;
}
```

Example code:

* Initialize a Skiplist
```C
skiplist_raw list;

skiplist_init(&list, cmp_func);
```

* Insert ```{1, 10}``` pair
```C
struct kv_node *node;

node = (struct kv_node*)malloc(sizeof(struct kv_node));
skiplist_init_node(&node->snode);

node->key = 1;
node->value = 10;
skiplist_insert(&list, &node->snode);
```

* Insert ```{2, 20}``` pair
```C
node = (struct kv_node*)malloc(sizeof(struct kv_node));
skiplist_init_node(&node->snode);

node->key = 2;
node->value = 20;
skiplist_insert(&list, &node->snode);
```

* Find the value corresponding to key ```1```
```C
struct kv_node query;
skiplist_node *cursor;

query.key = 1;
cursor = skiplist_find(&list, &query.snode);

// get 'node' from 'cursor'
node = _get_entry(cursor, struct kv_node, snode);
printf("%d\n", node->value);    // it will display 10
```

* Iteration
```C
cursor = skiplist_begin(&list);
while (cursor) {
    // get 'node' from 'cursor'
    node = _get_entry(cursor, struct kv_node, snode);

    // ... do something with 'node' ...

    // get next cursor
    cursor = skiplist_next(&list, cur);
}
```

* Remove the key-value pair corresponding to key ```1```
```C
query.key = 1;
cursor = skiplist_find(&list, &query.snode);
if (cursor) {
    // get 'node' from 'cursor'
    node = _get_entry(cursor, struct kv_node, snode);
    // remove from list
    skiplist_erase_node(&list, cursor);
    // free 'cursor' (i.e., node->snode)
    skiplist_free_node(cursor);
    // free 'node'
    free(node);
}
```

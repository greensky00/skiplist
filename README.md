Skiplist
--------
A generic Skiplist container C implementation, lock free for both multiple readers and writers. It can be used as a set or a map, containing any type of data.

It basically uses STL atomic variables with C++ compiler, but they can be switched to built-in GCC atomic operations when we compile it with pure C compiler.

This repository also contains STL-style lock-free `set` and `map` containers, based on Skiplist implementation. 


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

* Pure C

[examples/pure_c_example.c](examples/pure_c_example.c)

* C++ (STL-style `set` and `map`)

[examples/cpp_container_example.cc](examples/cpp_container_example.cc)


Benchmark results
-----------------
* Skiplist vs. STL set + STL mutex
* Single writer and multiple readers
* Randomly insert and read 100K integers

![alt text](https://github.com/greensky00/skiplist/blob/master/docs/swmr_graph.png "Throughput")

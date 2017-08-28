#include <stdio.h>

#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include "sl_map.h"
#include "test_common.h"

struct thread_args {
    thread_args() : mode(SKIPLIST), num(0), id(0), modulo(0),
                    duration_ms(0), op_count(0), temp(0),
                    sl(nullptr),
                    stdmap(nullptr), lock(nullptr) {}

    enum Mode {
        SKIPLIST = 0,
        MAP_MUTEX = 1,
        MAP_ONLY = 2
    } mode;
    int num;
    int id;
    int modulo;
    int duration_ms;
    int op_count;
    volatile uint64_t temp;
    sl_map<int, int>* sl;
    std::map<int, int>* stdmap;
    std::mutex* lock;
};

size_t num_primes(uint64_t number, size_t max_prime) {
    size_t ret = 0;
    for (size_t ii=2; ii<=max_prime; ++ii) {
        if (number % ii == 0) {
            number /= ii;
            ret++;
        }
    }
    return ret;
}

void reader(thread_args* args) {
    TestSuite::Timer timer(args->duration_ms);
    while (!timer.timeover()) {
        int r = rand() % args->num;
        int max_walks = 3;
        int walks = 0;

        if (args->mode == thread_args::SKIPLIST) {
            auto itr = args->sl->find(r);
            while (itr != args->sl->end()) {
                uint64_t number = itr->second;
                itr++;
                args->temp += num_primes(number, 10000);
                if (++walks >= max_walks) break;
            }

        } else if (args->mode == thread_args::MAP_MUTEX) {
            std::lock_guard<std::mutex> l(*args->lock);
            auto itr = args->stdmap->find(r);
            while (itr != args->stdmap->end()) {
                uint64_t number = itr->second;
                itr++;
                args->temp += num_primes(number, 10000);
                if (++walks >= max_walks) break;
            }

        } else  {
            auto itr = args->stdmap->find(r);
            while (itr != args->stdmap->end()) {
                uint64_t number = itr->second;
                itr++;
                args->temp += num_primes(number, 10000);
                if (++walks >= max_walks) break;
            }
        }
        args->op_count += max_walks;
    }
}

void writer(thread_args* args) {
    TestSuite::Timer timer(args->duration_ms);
    while (!timer.timeover()) {
        int r = rand() % (args->num / args->modulo);
        r *= args->modulo;
        r += args->id;

        if (args->mode == thread_args::SKIPLIST) {
            auto itr = args->sl->find(r);
            if (itr == args->sl->end()) {
                args->sl->insert(std::make_pair(r, r));
            } else {
                args->sl->erase(itr);
            }

        } else if (args->mode == thread_args::MAP_MUTEX) {
            std::lock_guard<std::mutex> l(*args->lock);
            auto itr = args->stdmap->find(r);
            if (itr == args->stdmap->end()) {
                args->stdmap->insert(std::make_pair(r, r));
            } else {
                args->stdmap->erase(itr);
            }

        } else  {
            auto itr = args->stdmap->find(r);
            if (itr == args->stdmap->end()) {
                args->stdmap->insert(std::make_pair(r, r));
            } else {
                args->stdmap->erase(itr);
            }
        }
        args->op_count++;
    }
}

int concurrent_test(int mode) {
    sl_map<int, int> sl;
    std::map<int, int> stdmap;
    std::mutex lock;
    int num = 10000000;
    int duration_ms = 5000;

    int num_readers = 4;
    thread_args r_args[num_readers];
    std::thread readers[num_readers];
    for (int i=0; i<num_readers; ++i) {
        r_args[i].mode = static_cast<thread_args::Mode>(mode);
        r_args[i].num = num;
        r_args[i].duration_ms = duration_ms;
        r_args[i].sl = &sl;
        r_args[i].stdmap = &stdmap;
        r_args[i].lock = &lock;
        readers[i] = std::thread(reader, &r_args[i]);
    }
    int num_writers = 4;
    thread_args w_args[num_writers];
    std::thread writers[num_writers];
    for (int i=0; i<num_writers; ++i) {
        w_args[i].mode = static_cast<thread_args::Mode>(mode);
        w_args[i].num = num;
        w_args[i].id = i;
        w_args[i].modulo = num_writers;
        w_args[i].duration_ms = duration_ms;
        w_args[i].sl = &sl;
        w_args[i].stdmap = &stdmap;
        w_args[i].lock = &lock;
        writers[i] = std::thread(writer, &w_args[i]);
    }

    int r_total = 0, w_total = 0;
    for (int i=0; i<num_readers; ++i) {
        readers[i].join();
        r_total += r_args[i].op_count;
    }
    for (int i=0; i<num_writers; ++i) {
        writers[i].join();
        w_total += w_args[i].op_count;
    }

    printf("read: %.1f ops/sec\n"
           "write: %.1f ops/sec\n"
           "total: %.1f ops/sec\n",
           (double)r_total * 1000.0 / duration_ms,
           (double)w_total * 1000.0 / duration_ms,
           (double)(r_total + w_total) * 1000.0 / duration_ms);

    return 0;
}

int main(int argc, char** argv) {
    TestSuite tt(argc, argv);

    std::vector<int> params = {0, 1, 2};
    tt.options.printTestMessage = true;
    tt.doTest("concurrent access comparison test", concurrent_test, TestRange<int>(params));

    return 0;
}

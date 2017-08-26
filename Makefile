LDFLAGS = -pthread
CFLAGS = \
	-g -D_GNU_SOURCE \
	-I. -I./src -I./debug -I./include -I./examples \

CFLAGS += -Wall
#CFLAGS += -O3

CXXFLAGS = $(CFLAGS) \
	--std=c++11 \


SKIPLIST = src/skiplist.o

TEST = tests/skiplist_test.o \
	   $(SKIPLIST)

ITR_TEST = \
	tests/mt_itr_write_erase_test.o \
	$(SKIPLIST)

CONTAINER_TEST = \
	tests/container_test.o \
	$(SKIPLIST)

PURE_C_EXAMPLE = \
	examples/pure_c_example.o \
	$(SKIPLIST)

CPP_CONTAINER_EXAMPLE = \
	examples/cpp_container_example.o \
	$(SKIPLIST)

PROGRAMS = \
	skiplist_test \
	itr_test \
	container_test \
	examples/pure_c_example \
	examples/cpp_container_example \


all: $(PROGRAMS)

skiplist_test: $(TEST)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

itr_test: $(ITR_TEST)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

container_test: $(CONTAINER_TEST)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

examples/pure_c_example: $(PURE_C_EXAMPLE)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

examples/cpp_container_example: $(CPP_CONTAINER_EXAMPLE)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(PROGRAMS) ./*.o ./*.so ./*/*.o ./*/*.so

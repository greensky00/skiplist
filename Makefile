LDFLAGS = -pthread
CFLAGS = \
	-g -D_GNU_SOURCE \
	-I. -I./src -I./debug -I./include -I./examples -I./tests \
	-fPIC \

CFLAGS += -Wall
#CFLAGS += -O3

CXXFLAGS = $(CFLAGS) \
	--std=c++11 \


SKIPLIST = src/skiplist.o
SHARED_LIB = libskiplist.so
STATIC_LIB = libskiplist.a

TEST = \
	tests/skiplist_test.o \
	$(STATIC_LIB) \

MT_TEST = \
	tests/mt_test.o \
	$(STATIC_LIB) \

STL_MAP_COMPARE = \
	tests/stl_map_compare.o \
	$(STATIC_LIB) \

CONTAINER_TEST = \
	tests/container_test.o \
	$(STATIC_LIB) \

PURE_C_EXAMPLE = \
	examples/pure_c_example.o \
	$(STATIC_LIB) \

CPP_CONTAINER_EXAMPLE = \
	examples/cpp_container_example.o \
	$(STATIC_LIB) \

PROGRAMS = \
	tests/skiplist_test \
	tests/mt_test \
	tests/container_test \
	tests/stl_map_compare \
	examples/pure_c_example \
	examples/cpp_container_example \
	libskiplist.so \
	libskiplist.a \

all: $(PROGRAMS)

libskiplist.so: $(SKIPLIST)
	$(CXX) $(CXXFLAGS) -shared $(LDBFALGS) -o $(SHARED_LIB) $(SKIPLIST)

libskiplist.a: $(SKIPLIST)
	ar rcs $(STATIC_LIB) $(SKIPLIST)

tests/skiplist_test: $(TEST)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

tests/mt_test: $(MT_TEST)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

tests/container_test: $(CONTAINER_TEST)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

tests/stl_map_compare: $(STL_MAP_COMPARE)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

examples/pure_c_example: $(PURE_C_EXAMPLE)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

examples/cpp_container_example: $(CPP_CONTAINER_EXAMPLE)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(PROGRAMS) ./*.o ./*.so ./*/*.o ./*/*.so

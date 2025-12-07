CXX = clang++
CXXFLAGS = -std=c++11 -g -W -Wall -Wextra -Wno-deprecated-declarations
CXXFLAGS_17 = -std=c++17 -g -W -Wall -Wextra
GTEST_FLAGS = -lgtest -lgtest_main -pthread

# Targets
ALLOCATOR_LIB = liballocator.dylib
SMALLOC_LIB = libsmalloc.dylib
C_TEST = test_allocator
CPP_TEST = allocator_tests

# Sources
ALLOCATOR_SRCS = allocator.cpp
SMALLOC_SRCS = allocator.cpp smalloc.cpp
C_TEST_SRCS = main.cpp smalloc.cpp allocator.cpp
CPP_TEST_SRCS = allocator_tests.cpp allocator.cpp

all: $(ALLOCATOR_LIB) $(SMALLOC_LIB) $(C_TEST) $(CPP_TEST)

# Libraries
$(ALLOCATOR_LIB): allocator.o
	$(CXX) -dynamiclib -o $@ $^

$(SMALLOC_LIB): allocator.o smalloc.o
	$(CXX) -dynamiclib -o $@ $^

# Test executables
$(C_TEST): main.o smalloc.o allocator.o
	$(CXX) -o $@ $^

$(CPP_TEST): allocator_tests.o allocator.o
	$(CXX) -o $@ $^ $(GTEST_FLAGS)

# Object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Special case for C++17 test file
allocator_tests.o: allocator_tests.cpp
	$(CXX) $(CXXFLAGS_17) -c $< -o $@

# Test targets
test: $(C_TEST)
	@echo "Running C-style function tests..."
	./$(C_TEST)

test-gtest: $(CPP_TEST)
	@echo "Running GoogleTest allocator tests..."
	./$(CPP_TEST)

test-all: test test-gtest
	@echo "All tests completed!"

clean:
	rm -f *.o $(ALLOCATOR_LIB) $(SMALLOC_LIB) $(C_TEST) $(CPP_TEST)

.PHONY: all test test-gtest test-all clean
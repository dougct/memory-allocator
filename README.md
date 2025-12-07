# Custom Memory Allocator

A custom memory allocator implementation with both C++ and C interfaces.

## How It Works

### Memory Management Strategy

The allocator uses a **block-based approach** with metadata tracking:

1. **Block Metadata**: Each allocated block has an associated `block_meta` structure containing:
   - `size`: Size of the allocated block in bytes
   - `free`: Boolean flag indicating if the block is available for reuse

2. **Memory Layout**:
   ```
   [block_meta][user data]
   ```
   When you call `malloc(100)`, the allocator requests `100 + sizeof(block_meta)` bytes from the system.

3. **Block Tracking**: All blocks are managed in a `std::list<block_meta*>` for efficient searching and management.

### Allocation Process

1. **Find Free Block**: Uses `std::find_if` with a lambda to search for a free block with sufficient size:
   ```cpp
   [size](const block_meta* block) {
       return block->free && block->size >= size;
   }
   ```

2. **Reuse or Request**:
   - If a suitable free block exists → mark it as used and return it
   - Otherwise → request new memory from the system via `mmap()`

3. **System Calls**: Uses `mmap()` instead of `sbrk()` for better portability and to avoid deprecation warnings.

### Memory Reclamation

- **free()**: Marks blocks as free but doesn't return memory to the system immediately
- **Destructor**: Automatically calls `munmap()` on all allocated blocks when the allocator is destroyed
- **Fragmentation Handling**: Free blocks can be reused by subsequent allocations

### C Interface Implementation

The C-style functions (`smalloc`, `sfree`, etc.) use a **Meyer's Singleton** pattern:
```cpp
static allocator& get_global_allocator() {
  static allocator instance;  // Thread-safe lazy initialization
  return instance;
}
```

This provides:
- Thread-safe initialization (C++11 guarantee)
- Automatic cleanup at program termination
- Global access without global constructor issues

## Architecture

### C++ Interface (allocator class)
- `allocator::malloc(size_t size)` - Allocate memory
- `allocator::free(void* ptr)` - Free allocated memory
- `allocator::realloc(void* ptr, size_t size)` - Reallocate memory
- `allocator::calloc(size_t nmemb, size_t size)` - Allocate zero-initialized memory

### C Interface (compatibility layer)
- `smalloc(size_t size)` - Allocate memory
- `sfree(void* ptr)` - Free allocated memory
- `srealloc(void* ptr, size_t size)` - Reallocate memory
- `scalloc(size_t nmemb, size_t size)` - Allocate zero-initialized memory

## Building

### C++ Allocator Library
```bash
make liballocator.dylib
```

### C Interface Library
```bash
make libsmalloc.dylib
```

### Build Everything (Libraries + Tests)
```bash
make all
```

## Testing

### Test C-Style Functions
```bash
make test
```

### Test C++ Allocator Class (GoogleTest)
```bash
make test-gtest
```

### Run All Tests
```bash
make test-all
```

### Clean Build Artifacts
```bash
make clean
```

## Files

- `allocator.h` - C++ allocator class interface
- `allocator.cpp` - C++ allocator implementation
- `smalloc.h` - C interface declarations
- `smalloc.cpp` - C interface implementation (uses C++ allocator)
- `main.cpp` - Test suite for C-style functions (smalloc, sfree, etc.)
- `allocator_tests.cpp` - GoogleTest suite for C++ allocator class

## Dependencies

- **GoogleTest**: Required for `allocator_tests.cpp`
  - Install on macOS: `brew install googletest`
  - Install on Ubuntu: `sudo apt-get install libgtest-dev`

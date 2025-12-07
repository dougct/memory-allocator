#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

// Stores meta information about each block of memory
struct block_meta {
  size_t size{0};
  bool free{true};
  block_meta* next{nullptr};
};

#define META_SIZE sizeof(struct block_meta)

class allocator {
 private:
  block_meta* head;

  block_meta* request_space(size_t size);
  block_meta* get_block_ptr(void* ptr);

 public:
  allocator();
  ~allocator();

  // Allocates size bytes and returns a pointer to the allocated memory. The
  // memory is not initialized.
  void* malloc(size_t size);

  // Frees the memory space pointed to by ptr, which must have been returned by
  // a previous call to malloc(), calloc() or realloc().
  void free(void* ptr);

  // Changes the size of the memory block pointed to by ptr to size bytes. The
  // contents will be unchanged in the range from the start of the region up to
  // the minimum of the old and new sizes.
  void* realloc(void* ptr, size_t size);

  // Allocates memory for an array of nmemb elements, each of size bytes, and
  // initializes all bytes to zero. The total allocation size is nmemb * size
  // bytes.
  void* calloc(size_t nmemb, size_t size);
};

#endif

#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits>

#include "allocator.h"

block_meta* allocator::request_space(size_t size) {
  block_meta* block = static_cast<block_meta*>(sbrk(size + META_SIZE));
  if (block == (void*)-1) {
    return nullptr;  // sbrk failed
  }

  block->size = size;
  block->free = false;

  return block;
}

block_meta* allocator::get_block_ptr(void* ptr) {
  return static_cast<block_meta*>(ptr) - 1;
}

// Allocator class method implementations

allocator::allocator() : head(nullptr) {}

allocator::~allocator() {
  // Note: sbrk allocations cannot be individually freed
  // The program break can only be moved, not individual blocks freed
}

void* allocator::malloc(size_t size) {
  if (size <= 0) {
    return nullptr;
  }

  // Try to find a free block with sufficient size
  block_meta* current = head;
  while (current) {
    if (current->free && current->size >= size) {
      current->free = false;
      return (current + 1);
    }
    current = current->next;
  }

  // No suitable free block found, request new space
  block_meta* block = request_space(size);
  if (!block) {
    return nullptr;
  }

  // Add the new block to the front of the linked list
  block->next = head;
  head = block;

  return (block + 1);
}

void allocator::free(void* ptr) {
  if (!ptr) {
    return;
  }

  block_meta* block_ptr = get_block_ptr(ptr);
  assert(block_ptr->free == false);
  block_ptr->free = true;
}

void* allocator::realloc(void* ptr, size_t size) {
  if (!ptr) {
    // nullptr, realloc should act like malloc
    return this->malloc(size);
  }

  block_meta* block_ptr = get_block_ptr(ptr);
  if (block_ptr->size >= size) {
    return ptr;
  }

  // Malloc new space, free old space, then copy data to new space
  void* new_ptr = this->malloc(size);
  if (!new_ptr) {
    return nullptr;
  }
  memcpy(new_ptr, ptr, block_ptr->size);
  this->free(ptr);

  return new_ptr;
}

void* allocator::calloc(size_t nmemb, size_t size) {
  if (size == 0) {
    return nullptr;
  }

  if (nmemb > std::numeric_limits<size_t>::max() / size) {
    return nullptr;  // Overflow would occur
  }

  const size_t total_size = nmemb * size;
  void* ptr = this->malloc(total_size);
  if (ptr) {
    memset(ptr, 0, total_size);
  }

  return ptr;
}

#include "allocator.h"

#include "smalloc.h"

// Meyer's Singleton
static allocator& get_global_allocator() {
  static allocator instance;
  return instance;
}

void* smalloc(size_t size) {
  return get_global_allocator().malloc(size);
}

void sfree(void* ptr) {
  get_global_allocator().free(ptr);
}

void* srealloc(void* ptr, size_t size) {
  return get_global_allocator().realloc(ptr, size);
}

void* scalloc(size_t nmemb, size_t size) {
  return get_global_allocator().calloc(nmemb, size);
}

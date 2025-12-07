#ifndef SMALLOC_H
#define SMALLOC_H

#include <stddef.h>

// C interface functions for memory allocation
void* smalloc(size_t size);
void sfree(void* ptr);
void* srealloc(void* ptr, size_t size);
void* scalloc(size_t nmemb, size_t size);

#endif

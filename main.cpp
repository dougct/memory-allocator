#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "smalloc.h"

void test_basic_allocation() {
  printf("Testing basic allocation...\n");

  void* ptr = smalloc(100);
  assert(ptr != nullptr);

  // Write to the memory to ensure it's valid
  memset(ptr, 0xAA, 100);

  sfree(ptr);

  printf("✓ Basic allocation test passed\n");
}

void test_zero_allocation() {
  printf("Testing zero allocation...\n");

  void* ptr = smalloc(0);
  assert(ptr == nullptr);

  printf("✓ Zero allocation test passed\n");
}

void test_null_free() {
  printf("Testing NULL free...\n");

  sfree(nullptr);  // Should not crash

  printf("✓ NULL free test passed\n");
}

void test_multiple_allocations() {
  printf("Testing multiple allocations...\n");

  void* ptrs[10];

  // Allocate multiple blocks
  for (int i = 0; i < 10; i++) {
    ptrs[i] = smalloc(50 + i * 10);
    assert(ptrs[i] != NULL);

    // Write pattern to each block
    memset(ptrs[i], i, 50 + i * 10);
  }

  // Verify patterns
  for (int i = 0; i < 10; i++) {
    unsigned char* data = static_cast<unsigned char*>(ptrs[i]);
    for (int j = 0; j < 50 + i * 10; j++) {
      assert(data[j] == i);
    }
  }

  // Free all blocks
  for (int i = 0; i < 10; i++) {
    sfree(ptrs[i]);
  }

  printf("✓ Multiple allocations test passed\n");
}

void test_different_block_sizes() {
  printf("Testing different block sizes...\n");

  size_t sizes[] = {1, 8, 16, 32, 64, 128, 256, 512, 1024, 4096, 8192};
  int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

  for (int i = 0; i < num_sizes; i++) {
    void* ptr = smalloc(sizes[i]);
    assert(ptr != nullptr);

    // Write and verify pattern
    memset(ptr, 0x55, sizes[i]);
    unsigned char* data = static_cast<unsigned char*>(ptr);
    for (size_t j = 0; j < sizes[i]; j++) {
      assert(data[j] == 0x55);
    }

    sfree(ptr);
    printf("  ✓ Size %zu bytes\n", sizes[i]);
  }

  printf("✓ Different block sizes test passed\n");
}

void test_fragmentation() {
  printf("Testing fragmentation and reuse...\n");

  // Allocate alternating pattern
  void* ptr1 = smalloc(100);
  void* ptr2 = smalloc(200);
  void* ptr3 = smalloc(100);
  void* ptr4 = smalloc(200);

  assert(ptr1 && ptr2 && ptr3 && ptr4);

  // Free alternating blocks
  sfree(ptr1);
  sfree(ptr3);

  // Allocate blocks that should reuse freed space
  void* ptr5 = smalloc(50);  // Should fit in ptr1's space
  void* ptr6 = smalloc(80);  // Should fit in ptr3's space

  assert(ptr5 && ptr6);

  // Clean up
  sfree(ptr2);
  sfree(ptr4);
  sfree(ptr5);
  sfree(ptr6);

  printf("✓ Fragmentation test passed\n");
}

void test_calloc() {
  printf("Testing calloc...\n");

  int* arr = static_cast<int*>(scalloc(10, sizeof(int)));
  assert(arr != nullptr);

  // Verify all elements are zero
  for (int i = 0; i < 10; i++) {
    assert(arr[i] == 0);
  }

  // Write some data
  for (int i = 0; i < 10; i++) {
    arr[i] = i * 2;
  }

  // Verify data
  for (int i = 0; i < 10; i++) {
    assert(arr[i] == i * 2);
  }

  sfree(arr);

  printf("✓ calloc test passed\n");
}

void test_realloc() {
  printf("Testing realloc...\n");

  // Test realloc with NULL (should act like malloc)
  void* ptr = srealloc(nullptr, 100);
  assert(ptr != nullptr);
  memset(ptr, 0xBB, 100);

  // Test expanding
  ptr = srealloc(ptr, 200);
  assert(ptr != nullptr);

  // Verify original data is preserved
  unsigned char* data = static_cast<unsigned char*>(ptr);
  for (int i = 0; i < 100; i++) {
    assert(data[i] == 0xBB);
  }

  // Test shrinking (should return same pointer)
  ptr = srealloc(ptr, 50);
  // Note: implementation may return same pointer for shrinking
  assert(ptr != nullptr);

  sfree(ptr);

  printf("✓ realloc test passed\n");
}

void test_large_allocation() {
  printf("Testing large allocation...\n");

  size_t large_size = 1024 * 1024;  // 1MB
  void* ptr = smalloc(large_size);
  assert(ptr != nullptr);

  // Write to first and last bytes
  unsigned char* data = static_cast<unsigned char*>(ptr);
  data[0] = 0xCC;
  data[large_size - 1] = 0xDD;

  // Verify
  assert(data[0] == 0xCC);
  assert(data[large_size - 1] == 0xDD);

  sfree(ptr);

  printf("✓ Large allocation test passed\n");
}

void test_stress() {
  printf("Testing stress (many allocations)...\n");

  const int num_allocs = 1000;
  void* ptrs[num_allocs];

  // Allocate many small blocks
  for (int i = 0; i < num_allocs; i++) {
    ptrs[i] = smalloc(i % 100 + 1);
    assert(ptrs[i] != NULL);
  }

  // Free every other block
  for (int i = 0; i < num_allocs; i += 2) {
    sfree(ptrs[i]);
    ptrs[i] = nullptr;
  }

  // Allocate new blocks in freed spaces
  for (int i = 0; i < num_allocs; i += 2) {
    ptrs[i] = smalloc(i % 50 + 1);
    assert(ptrs[i] != NULL);
  }

  // Free all remaining blocks
  for (int i = 0; i < num_allocs; i++) {
    if (ptrs[i]) {
      sfree(ptrs[i]);
    }
  }

  printf("✓ Stress test passed\n");
}

int main() {
  printf("Starting C-style smalloc tests...\n\n");

  test_basic_allocation();
  test_zero_allocation();
  test_null_free();
  test_multiple_allocations();
  test_different_block_sizes();
  test_fragmentation();
  test_calloc();
  test_realloc();
  test_large_allocation();
  test_stress();

  printf("\n🎉 All C-style function tests passed!\n");
  return 0;
}

#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include "allocator.h"

class AllocatorTest : public ::testing::Test {
 protected:
  void SetUp() override { allocator_ = std::make_unique<allocator>(); }

  std::unique_ptr<allocator> allocator_;
};

TEST_F(AllocatorTest, BasicAllocation) {
  void* ptr = allocator_->malloc(100);
  ASSERT_NE(ptr, nullptr);
  // Write to the memory to ensure it's valid
  memset(ptr, 0xAA, 100);
  allocator_->free(ptr);
}

TEST_F(AllocatorTest, ZeroAllocation) {
  void* ptr = allocator_->malloc(0);
  EXPECT_EQ(ptr, nullptr);
}

TEST_F(AllocatorTest, NullFree) {
  // Should not crash
  EXPECT_NO_THROW(allocator_->free(nullptr));
}

TEST_F(AllocatorTest, MultipleAllocations) {
  const int num_allocs = 10;
  void* ptrs[num_allocs];

  // Allocate multiple blocks
  for (int i = 0; i < num_allocs; i++) {
    ptrs[i] = allocator_->malloc(50 + i * 10);
    ASSERT_NE(ptrs[i], nullptr);

    // Write pattern to each block
    memset(ptrs[i], i, 50 + i * 10);
  }

  // Verify patterns
  for (int i = 0; i < num_allocs; i++) {
    unsigned char* data = static_cast<unsigned char*>(ptrs[i]);
    for (int j = 0; j < 50 + i * 10; j++) {
      EXPECT_EQ(data[j], i);
    }
  }

  // Free all blocks
  for (int i = 0; i < num_allocs; i++) {
    allocator_->free(ptrs[i]);
  }
}

TEST_F(AllocatorTest, DifferentBlockSizes) {
  size_t sizes[] = {1, 8, 16, 32, 64, 128, 256, 512, 1024, 4096, 8192};
  int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

  for (int i = 0; i < num_sizes; i++) {
    void* ptr = allocator_->malloc(sizes[i]);
    ASSERT_NE(ptr, nullptr) << "Failed to allocate " << sizes[i] << " bytes";

    // Write and verify pattern
    memset(ptr, 0x55, sizes[i]);
    unsigned char* data = static_cast<unsigned char*>(ptr);
    for (size_t j = 0; j < sizes[i]; j++) {
      EXPECT_EQ(data[j], 0x55);
    }

    allocator_->free(ptr);
  }
}

TEST_F(AllocatorTest, FragmentationAndReuse) {
  // Allocate alternating pattern
  void* ptr1 = allocator_->malloc(100);
  void* ptr2 = allocator_->malloc(200);
  void* ptr3 = allocator_->malloc(100);
  void* ptr4 = allocator_->malloc(200);

  ASSERT_NE(ptr1, nullptr);
  ASSERT_NE(ptr2, nullptr);
  ASSERT_NE(ptr3, nullptr);
  ASSERT_NE(ptr4, nullptr);

  // Free alternating blocks
  allocator_->free(ptr1);
  allocator_->free(ptr3);

  // Allocate blocks that should reuse freed space
  void* ptr5 = allocator_->malloc(50);  // Should fit in ptr1's space
  void* ptr6 = allocator_->malloc(80);  // Should fit in ptr3's space

  EXPECT_NE(ptr5, nullptr);
  EXPECT_NE(ptr6, nullptr);

  // Clean up
  allocator_->free(ptr2);
  allocator_->free(ptr4);
  allocator_->free(ptr5);
  allocator_->free(ptr6);
}

TEST_F(AllocatorTest, Calloc) {
  int* arr = static_cast<int*>(allocator_->calloc(10, sizeof(int)));
  ASSERT_NE(arr, nullptr);

  // Verify all elements are zero
  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(arr[i], 0);
  }

  // Write some data
  for (int i = 0; i < 10; i++) {
    arr[i] = i * 2;
  }

  // Verify data
  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(arr[i], i * 2);
  }

  allocator_->free(arr);
}

TEST_F(AllocatorTest, Realloc) {
  // Test realloc with nullptr (should act like malloc)
  void* ptr = allocator_->realloc(nullptr, 100);
  ASSERT_NE(ptr, nullptr);
  memset(ptr, 0xBB, 100);

  // Test expanding
  ptr = allocator_->realloc(ptr, 200);
  ASSERT_NE(ptr, nullptr);

  // Verify original data is preserved
  unsigned char* data = static_cast<unsigned char*>(ptr);
  for (int i = 0; i < 100; i++) {
    EXPECT_EQ(data[i], 0xBB);
  }

  // Test shrinking
  ptr = allocator_->realloc(ptr, 50);
  EXPECT_NE(ptr, nullptr);

  allocator_->free(ptr);
}

TEST_F(AllocatorTest, LargeAllocation) {
  size_t large_size = 1024 * 1024;  // 1MB
  void* ptr = allocator_->malloc(large_size);
  ASSERT_NE(ptr, nullptr);

  // Write to first and last bytes
  unsigned char* data = static_cast<unsigned char*>(ptr);
  data[0] = 0xCC;
  data[large_size - 1] = 0xDD;

  // Verify
  EXPECT_EQ(data[0], 0xCC);
  EXPECT_EQ(data[large_size - 1], 0xDD);

  allocator_->free(ptr);
}

TEST_F(AllocatorTest, StressTest) {
  const int num_allocs = 1000;
  void* ptrs[num_allocs];

  // Allocate many small blocks
  for (int i = 0; i < num_allocs; i++) {
    ptrs[i] = allocator_->malloc(i % 100 + 1);
    ASSERT_NE(ptrs[i], nullptr);
  }

  // Free every other block
  for (int i = 0; i < num_allocs; i += 2) {
    allocator_->free(ptrs[i]);
    ptrs[i] = nullptr;
  }

  // Allocate new blocks in freed spaces
  for (int i = 0; i < num_allocs; i += 2) {
    ptrs[i] = allocator_->malloc(i % 50 + 1);
    ASSERT_NE(ptrs[i], nullptr);
  }

  // Free all remaining blocks
  for (int i = 0; i < num_allocs; i++) {
    if (ptrs[i]) {
      allocator_->free(ptrs[i]);
    }
  }
}

// Test multiple allocator instances
TEST(AllocatorMultipleInstancesTest, IndependentAllocators) {
  allocator alloc1;
  allocator alloc2;

  void* ptr1 = alloc1.malloc(100);
  void* ptr2 = alloc2.malloc(100);

  EXPECT_NE(ptr1, nullptr);
  EXPECT_NE(ptr2, nullptr);
  EXPECT_NE(ptr1, ptr2);  // Should be different memory regions

  alloc1.free(ptr1);
  alloc2.free(ptr2);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

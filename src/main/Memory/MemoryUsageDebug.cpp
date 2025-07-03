#ifndef MEMORY_USAGE_CPP
#define MEMORY_USAGE_CPP

#include "AllocationMetrics.h"

#include <memory>

// ReSharper disable CppParameterNamesMismatch
// void* operator new(const size_t size) noexcept { // NOLINT(*-new-delete-overloads)
//   Memory::s_AllocationMetrics.totalAllocated += size;
//   return malloc(size);
// }
//
// void* operator new[](const size_t size) noexcept {
//     Memory::s_AllocationMetrics.totalAllocated += size;
//     return malloc(size);
// }
//
// void operator delete(void* memory, const size_t size) noexcept {
//     Memory::s_AllocationMetrics.totalFreed += size;
//     free(memory);
// }
//
// void operator delete[](void* memory, const size_t size) noexcept {
//     Memory::s_AllocationMetrics.totalFreed += size;
//     free(memory);
// }
// ReSharper restore CppParameterNamesMismatch

#endif //MEMORY_USAGE_CPP

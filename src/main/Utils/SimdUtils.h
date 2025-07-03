#ifndef SIMD_UTILS_H
#define SIMD_UTILS_H

#if defined(__AVX__) && (not defined(_MSC_VER) || (defined(_MSC_VER) && defined(_M_X64)))

#define HAS_SIMD

#include <immintrin.h> // AVX intrinsics
#include <cstdint>

namespace Simd {
    inline void copy_bits(uint8_t *dst, const uint8_t *src, const size_t count) noexcept {
        constexpr size_t SIMDWidth = 32; // AVX processes 256 bits (32 bytes) at a time
        const size_t simdCount = count / SIMDWidth;

        for (size_t i = 0; i < simdCount; ++i) {
            const __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(src + i * SIMDWidth));
            _mm256_storeu_si256(reinterpret_cast<__m256i *>(dst + i * SIMDWidth), chunk);
        }

        // Handle remaining bits
        const size_t remaining = count % SIMDWidth;
        const size_t offset = simdCount * SIMDWidth;
        for (size_t i = 0; i < remaining; ++i) dst[offset + i] = src[offset + i];
    }
}

#endif

#endif //SIMD_UTILS_H

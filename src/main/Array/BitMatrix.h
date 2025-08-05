#ifndef BITMATRIX_H
#define BITMATRIX_H

#include <cassert>

#include "Array/BitArray.h"

namespace BitMatrix {
    typedef uint32_t dimension_size_t;

    using array_size_t = BitArray::array_size_t;

    class BitMatrix {
    public:
        explicit BitMatrix(const dimension_size_t xSize, const dimension_size_t ySize) noexcept : m_XSize(xSize),
            m_YSize(ySize),
            m_Array(xSize * ySize) { }

        virtual ~BitMatrix() noexcept = default;

        [[nodiscard]] array_size_t // ReSharper disable once CppMemberFunctionMayBeStatic
        offsetX(const dimension_size_t y) const noexcept { // NOLINT(*-convert-member-functions-to-static)
            return y;
        }

        [[nodiscard]] array_size_t offsetY(const dimension_size_t x) const noexcept { return x * m_YSize; }

        void assign(const dimension_size_t x, const dimension_size_t y, const uint8_t value) noexcept {
            m_Array.assign(index(x, y), value);
        }

        void assign(const dimension_size_t x, const dimension_size_t y, const int8_t value) noexcept {
            m_Array.assign(index(x, y), value);
        }

        void assign(const dimension_size_t x, const dimension_size_t y, const uint16_t value) noexcept {
            m_Array.assign(index(x, y), value);
        }

        void assign(const dimension_size_t x, const dimension_size_t y, const int16_t value) noexcept {
            m_Array.assign(index(x, y), value);
        }

        void assign(const dimension_size_t x, const dimension_size_t y, const uint32_t value) noexcept {
            m_Array.assign(index(x, y), value);
        }

        void assign(const dimension_size_t x, const dimension_size_t y, const int32_t value) noexcept {
            m_Array.assign(index(x, y), value);
        }

        void assign(const dimension_size_t x, const dimension_size_t y, const uint64_t value) noexcept {
            m_Array.assign(index(x, y), value);
        }

        void assign(const dimension_size_t x, const dimension_size_t y, const int64_t value) noexcept {
            m_Array.assign(index(x, y), value);
        }

        void set(const dimension_size_t x, const dimension_size_t y) noexcept { m_Array.set(index(x, y)); }
        void clear(const dimension_size_t x, const dimension_size_t y) noexcept { m_Array.clear(index(x, y)); }

        [[nodiscard]] uint8_t get(const dimension_size_t x, const dimension_size_t y) const noexcept {
            return m_Array.get(index(x, y));
        }

        uint8_t operator[](const dimension_size_t x, const dimension_size_t y) const noexcept { return m_Array[index(x, y)]; }

    protected:
        dimension_size_t m_XSize, m_YSize;
        BitArray::BitArray m_Array;

        [[nodiscard]] array_size_t index(const dimension_size_t x, const dimension_size_t y) const noexcept {
            assert(x < m_XSize && "X must be less than the X dimension size.");
            assert(y < m_YSize && "Y must be less than the Y dimension size.");
            return x * m_YSize + y;
        }
    };

    class BitMatrix3D {
    public:
        explicit BitMatrix3D(const dimension_size_t xSize, const dimension_size_t ySize, const dimension_size_t zSize) noexcept :
            m_XSize(xSize),
            m_YSize(ySize),
            m_ZSize(zSize),
            m_Array(xSize * ySize * zSize) { }

        [[nodiscard]] array_size_t offsetX(const dimension_size_t y, const dimension_size_t z) const noexcept {
            return y * m_ZSize + z;
        }

        [[nodiscard]] array_size_t offsetY(const dimension_size_t x, const dimension_size_t z) const noexcept {
            return x * m_YSize * m_ZSize + z;
        }

        [[nodiscard]] array_size_t offsetZ(const dimension_size_t x, const dimension_size_t y) const noexcept {
            return x * m_YSize * m_ZSize + y * m_ZSize;
        }

        void assign(const dimension_size_t x, const dimension_size_t y, const dimension_size_t z, const uint8_t value) noexcept {
            m_Array.assign(index(x, y, z), value);
        }

        void assign(const dimension_size_t x, const dimension_size_t y, const dimension_size_t z, const int8_t value) noexcept {
            m_Array.assign(index(x, y, z), value);
        }

        void assign(const dimension_size_t x, const dimension_size_t y, const dimension_size_t z,
                    const uint16_t value) noexcept { m_Array.assign(index(x, y, z), value); }

        void assign(const dimension_size_t x, const dimension_size_t y, const dimension_size_t z, const int16_t value) noexcept {
            m_Array.assign(index(x, y, z), value);
        }

        void assign(const dimension_size_t x, const dimension_size_t y, const dimension_size_t z,
                    const uint32_t value) noexcept { m_Array.assign(index(x, y, z), value); }

        void assign(const dimension_size_t x, const dimension_size_t y, const dimension_size_t z, const int32_t value) noexcept {
            m_Array.assign(index(x, y, z), value);
        }

        void assign(const dimension_size_t x, const dimension_size_t y, const dimension_size_t z,
                    const uint64_t value) noexcept { m_Array.assign(index(x, y, z), value); }

        void assign(const dimension_size_t x, const dimension_size_t y, const dimension_size_t z, const int64_t value) noexcept {
            m_Array.assign(index(x, y, z), value);
        }

        void set(const dimension_size_t x, const dimension_size_t y, const dimension_size_t z) noexcept {
            m_Array.set(index(x, y, z));
        }

        void clear(const dimension_size_t x, const dimension_size_t y, const dimension_size_t z) noexcept {
            m_Array.clear(index(x, y, z));
        }

        [[nodiscard]] uint8_t get(const dimension_size_t x, const dimension_size_t y, const dimension_size_t z) const noexcept {
            return m_Array.get(index(x, y, z));
        }

        uint8_t operator[](const dimension_size_t x, const dimension_size_t y, const dimension_size_t z) const noexcept {
            return m_Array[index(x, y, z)];
        }

        /**
         * @param dst Destination array.
         * @param srcOffset Source array offset.
         */
        void validateZ(BitArray::BitArray& dst, const dimension_size_t srcOffset) const noexcept {
            assert(srcOffset <= m_Array.m_Size && "Offset (srcOffset) should not exceed total matrix size.");
            const size_t iterationCount = dst.m_Size >> 6; // 64 == 2^6
            assert(dst.m_Size <= m_ZSize && "Destination (dst) size should not exceed Z axis size.");
            for (size_t i = 0; i < iterationCount; ++i) {
                const uint64_t srcWord = m_Array.word(srcOffset + (i << 6));
                const uint64_t dstWord = dst.word(i << 6);
                const uint64_t valWord = ~srcWord & dstWord;
                dst.assignWord(i << 6, valWord);
            }
            if (const auto remainingBits = static_cast<uint8_t>(dst.m_Size & 63); remainingBits > 0) {
                const uint64_t srcWord = m_Array.wordn(srcOffset + (iterationCount << 6), remainingBits);
                const uint64_t dstWord = dst.wordn(iterationCount << 6, remainingBits);
                const uint64_t valWord = ~srcWord & dstWord;
                dst.assignWord(iterationCount << 6, valWord);
            }
        }

    protected:
        dimension_size_t m_XSize, m_YSize, m_ZSize;
        BitArray::BitArray m_Array;

        [[nodiscard]] constexpr array_size_t offset(const dimension_size_t x, const dimension_size_t y) const noexcept {
            assert(x < m_XSize && "X must be less than the X dimension size.");
            assert(y < m_YSize && "Y must be less than the Y dimension size.");
            return x * m_YSize * m_ZSize + y * m_ZSize;
        }

        [[nodiscard]] constexpr array_size_t index(const dimension_size_t x, const dimension_size_t y,
                                                   const dimension_size_t z) const noexcept {
            assert(z < m_ZSize && "Z must be less than the Z dimension size.");
            return offset(x, y) + z;
        }
    };

    BitMatrix createMatrix(array_size_t xSize, array_size_t ySize) noexcept;
    BitMatrix3D createMatrix3D(array_size_t xSize, array_size_t ySize, array_size_t zSize) noexcept;
}

#endif //BITMATRIX_H

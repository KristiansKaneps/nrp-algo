#ifndef BITSQUAREMATRIX_H
#define BITSQUAREMATRIX_H

#include <cassert>

#include "Array/BitArray.h"

namespace BitMatrix {
    typedef uint32_t dimension_size_t;

    using array_size_t = BitArray::array_size_t;

    class BitSquareMatrix {
    public:
        explicit BitSquareMatrix(const dimension_size_t dimensionSize) noexcept : m_DimensionSize(dimensionSize),
                                                                         m_Array(dimensionSize * dimensionSize) { }

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
        dimension_size_t m_DimensionSize;
        BitArray::BitArray m_Array;

        [[nodiscard]] array_size_t index(const dimension_size_t x, const dimension_size_t y) const noexcept {
            assert(x < m_DimensionSize && "X must be less than the dimension size.");
            assert(y < m_DimensionSize && "Y must be less than the dimension size.");
            return x * m_DimensionSize + y;
        }
    };

    BitSquareMatrix createSquareMatrix(array_size_t dimensionSize) noexcept;
    BitSquareMatrix createIdentitySquareMatrix(array_size_t dimensionSize) noexcept;
}

#endif //BITSQUAREMATRIX_H

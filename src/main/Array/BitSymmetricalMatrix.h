#ifndef BITSYMMETRICALMATRIX_H
#define BITSYMMETRICALMATRIX_H

#include <cassert>

#include "Array/BitArray.h"

namespace BitMatrix {
    typedef uint32_t dimension_size_t;

    using array_size_t = BitArray::array_size_t;

    class BitSymmetricalMatrix {
    public:
        explicit BitSymmetricalMatrix(const dimension_size_t dimensionSize) noexcept : m_DimensionSize(dimensionSize),
                                                                              m_Array(
                                                                                  dimensionSize * (dimensionSize + 1) +
                                                                                  1 >> 1) { }

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
            return x <= y
                       ? (x * (2 * m_DimensionSize - x + 1) >> 1) + (y - x)
                       : (y * (2 * m_DimensionSize - y + 1) >> 1) + (x - y);
        }

        [[nodiscard]] std::pair<dimension_size_t, dimension_size_t> invertIndex(const array_size_t index) const noexcept {
            dimension_size_t x = 0;
            while ((x * (2 * m_DimensionSize - x + 1)) >> 1 <= index) ++x;
            --x;
            array_size_t offset = (x * (2 * m_DimensionSize - x + 1)) >> 1;
            dimension_size_t y = x + (index - offset);
            return {x, y};
        }
    };

    BitSymmetricalMatrix createSymmetricalMatrix(array_size_t dimensionSize) noexcept;
    BitSymmetricalMatrix createIdentitySymmetricalMatrix(array_size_t dimensionSize) noexcept;
}

#endif //BITSYMMETRICALMATRIX_H

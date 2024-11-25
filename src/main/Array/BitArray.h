#ifndef BITARRAY_H
#define BITARRAY_H

#include <cassert>
#include <string>
#include <vector>

#ifndef _WIN32
#include "bit_array.h"
#else
#error Not yet implemented on Windows.
#endif

namespace BitMatrix {
    class BitMatrix;
    class BitMatrix3D;
    class BitSquareMatrix;
    class BitSymmetricalMatrix;
}

namespace BitArray {
    typedef uint64_t array_size_t;

    static inline constexpr char BIT_CHAR_REPRESENTATIONS[] = {'0', '1'};

    class BitArrayInterface {
    public:
        explicit BitArrayInterface(const array_size_t size) : m_Size(size) { }

        virtual ~BitArrayInterface() = default;

        [[nodiscard]] array_size_t size() const { return m_Size; }

        virtual std::string getStringRepresentation() const { // NOLINT(*-use-nodiscard)
            std::string str;
            str.reserve(m_Size);
            for (array_size_t i = 0; i < m_Size; ++i)
                str.push_back(getCharRepresentation(i));
            return str;
        }

        virtual void setAll() = 0;

        virtual void assign(array_size_t index, bool value) = 0;
        virtual void assign(array_size_t index, uint8_t value) = 0;
        virtual void assign(array_size_t index, int8_t value) = 0;
        virtual void assign(array_size_t index, uint16_t value) = 0;
        virtual void assign(array_size_t index, int16_t value) = 0;
        virtual void assign(array_size_t index, uint32_t value) = 0;
        virtual void assign(array_size_t index, int32_t value) = 0;
        virtual void assign(array_size_t index, uint64_t value) = 0;
        virtual void assign(array_size_t index, int64_t value) = 0;
        virtual void set(array_size_t index) = 0;
        virtual void clear(array_size_t index) = 0;

        virtual void assignWord(array_size_t index, uint64_t word) = 0;

        [[nodiscard]] virtual uint8_t get(array_size_t index) const = 0;

        virtual uint8_t operator[](const array_size_t index) const { return get(index); }

        [[nodiscard]] virtual uint64_t word(array_size_t index) const = 0;
        [[nodiscard]] virtual uint64_t wordn(array_size_t index, uint8_t n) const = 0;

        char getCharRepresentation(const array_size_t index) const { // NOLINT(*-use-nodiscard)
            return BIT_CHAR_REPRESENTATIONS[get(index)];
        }

        virtual void copyTo(BitArrayInterface& dst, array_size_t srcOffset, array_size_t srcStride,
                            array_size_t dstOffset) const = 0;

        virtual void random(float probability) = 0;
        virtual void random() { random(0.5f); }

        [[nodiscard]] virtual bool test(array_size_t index, array_size_t length) const = 0;

        [[nodiscard]] virtual array_size_t count() const = 0;

    protected:
        const array_size_t m_Size;

    private:
        friend class BitArray;
        friend class BitMatrix::BitSymmetricalMatrix;
        friend class BitMatrix::BitSquareMatrix;
        friend class BitMatrix::BitMatrix;
        friend class BitMatrix::BitMatrix3D;
    };

    #ifndef _WIN32
    class BitArray final : public BitArrayInterface {
    public:
        explicit BitArray(const array_size_t size) : BitArrayInterface(size) { m_Array = bit_array_create(m_Size); }

        explicit BitArray(const array_size_t size, const bool allSet) : BitArrayInterface(size) {
            m_Array = bit_array_create(m_Size);
            if (allSet) bit_array_set_all(m_Array);
            // else bit_array_clear_all(m_Array);
        }

        BitArray(const BitArray& other) : BitArrayInterface(other.m_Size) {
            m_Array = bit_array_create(m_Size);
            bit_array_copy_all(m_Array, other.m_Array);
        }

        ~BitArray() override { bit_array_free(m_Array); }

        [[nodiscard]] constexpr BIT_ARRAY *getUnderlyingImplementation() const { return m_Array; }

        // Does not work as expected:
        // std::string getStringRepresentation() const override { // NOLINT(*-use-nodiscard)
        //     // std::string str(m_Size, '\0');
        //     // bit_array_word2str(m_Array, m_Size, str.data());
        //     // return str;
        // }

        void setAll() override { bit_array_set_all(m_Array); }

        void assign(const array_size_t index, const bool value) override {
            bit_array_assign(m_Array, index, static_cast<uint8_t>(value));
        }

        void assign(const array_size_t index, const uint8_t value) override {
            bit_array_assign(m_Array, index, value & 0b1);
        }

        void assign(const array_size_t index, const int8_t value) override {
            bit_array_assign(m_Array, index, value & 0b1);
        }

        void assign(const array_size_t index, const uint16_t value) override {
            bit_array_assign(m_Array, index, value & 0b1);
        }

        void assign(const array_size_t index, const int16_t value) override {
            bit_array_assign(m_Array, index, value & 0b1);
        }

        void assign(const array_size_t index, const uint32_t value) override {
            bit_array_assign(m_Array, index, value & 0b1);
        }

        void assign(const array_size_t index, const int32_t value) override {
            bit_array_assign(m_Array, index, value & 0b1);
        }

        void assign(const array_size_t index, const uint64_t value) override {
            bit_array_assign(m_Array, index, value & 0b1);
        }

        void assign(const array_size_t index, const int64_t value) override {
            bit_array_assign(m_Array, index, value & 0b1);
        }

        void set(const array_size_t index) override { bit_array_set(m_Array, index); }
        void clear(const array_size_t index) override { bit_array_clear(m_Array, index); }

        void assignWord(const array_size_t index, const uint64_t word) override {
            bit_array_set_word64(m_Array, index, word);
        }

        [[nodiscard]] uint8_t get(const array_size_t index) const override {
            return static_cast<uint8_t>(bit_array_get_bit(m_Array, index));
        }

        uint8_t operator[](const array_size_t index) const override {
            return static_cast<uint8_t>(bit_array_get_bit(m_Array, index));
        }

        [[nodiscard]] uint64_t word(const array_size_t index) const override {
            return bit_array_get_word64(m_Array, index);
        }

        [[nodiscard]] uint64_t wordn(const array_size_t index, const uint8_t n) const override {
            assert(n <= 64 && "Max word length is 64 bits");
            return bit_array_get_wordn(m_Array, index, n);
        }

        void copyTo(BitArrayInterface& other, const array_size_t srcOffset, const array_size_t srcStride,
                    const array_size_t dstOffset) const override {
            assert(srcStride > 0 && "Stride must be larger than 0");
            auto& array = static_cast<BitArray&>(other); // NOLINT(*-pro-type-static-cast-downcast)

            if (srcStride == 1) {
                bit_array_copy(array.m_Array, dstOffset, m_Array, srcOffset, other.m_Size - dstOffset);
            } else {
                array_size_t dstIdx = dstOffset;
                for (array_size_t srcIdx = srcOffset; srcIdx < m_Size && dstIdx < other.m_Size; srcIdx += srcStride) {
                    bit_array_copy(array.m_Array, dstIdx++, m_Array, srcIdx, 1);
                }
            }
        }

        void random(const float probability) override { bit_array_random(m_Array, probability); }
        void random() override { bit_array_random(m_Array, 0.5f); }

        [[nodiscard]] bool test(const array_size_t index, const array_size_t length) const override {
            assert(index + length <= m_Size && "Parameter (index and length) sum should not exceed array length.");
            const size_t iterationCount = length >> 6; // 64 == 2^6
            for (size_t i = 0; i < iterationCount; ++i) {
                if (bit_array_get_word64(m_Array, index + (i << 6))) return true;
            }
            const auto remainingBits = static_cast<int8_t>(length & 63);
            return remainingBits > 0 && bit_array_get_wordn(m_Array, index + (iterationCount << 6), remainingBits);
        }

        [[nodiscard]] array_size_t count() const override { return bit_array_num_bits_set(m_Array); }

        void collectTestIndices(const BitArray& dst, const array_size_t offset,
                                std::vector<array_size_t>& result) const {
            const size_t iterationCount = dst.m_Size >> 6; // 64 == 2^6
            if (result.capacity() == 0) [[unlikely]] result.reserve((dst.m_Size >> 2) + 1);
            for (size_t i = 0; i < iterationCount; ++i) {
                uint64_t word = bit_array_get_word64(m_Array, offset + (i << 6));
                for (uint8_t j = 0; j < 64; ++j) {
                    if (word & 1 << j == 0) continue;
                    const array_size_t index = i << 6 | j;
                    result.push_back(index);
                }
            }
            if (const auto remainingBits = static_cast<int8_t>(dst.m_Size & 63); remainingBits > 0) {
                uint64_t word = bit_array_get_wordn(m_Array, iterationCount << 6, remainingBits);
                for (uint8_t j = 0; j < remainingBits; ++j) {
                    if (word & 1 << j == 0) continue;
                    const array_size_t index = iterationCount << 6 | j;
                    result.push_back(index);
                }
            }
        }

    protected:
        BIT_ARRAY *m_Array;
    };
    #else
    class BitArray final : public BitArrayInterface {
    public:
        explicit BitArray(const array_size_t size) : BitArrayInterface(size) {

        }

        BitArray(const BitArray& other) : BitArrayInterface(other.m_Size) {

        }

        ~BitArray() override {

        }
    protected:

    };
    #endif

    inline std::ostream& operator<<(std::ostream& out, const BitArrayInterface& array) {
        out << array.getStringRepresentation();
        return out;
    }
}

#endif //BITARRAY_H

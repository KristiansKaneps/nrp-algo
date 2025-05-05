#ifndef STATE_H
#define STATE_H

#include <cstdint>
#include <iostream>

#include "Time/Range.h"
#include "State/Axes.h"
#include "State/Size.h"

#include "Array/BitArray.h"

namespace State {
    template<typename X, typename Y, typename Z, typename W>
    class State {
    public:
        State(const Time::Range& range, const std::chrono::time_zone *timeZone, const Axes::Axis<X>* x, const Axes::Axis<Y>* y,
                      const Axes::Axis<Z>* z, const Axes::Axis<W>* w) : m_Size(x->size(), y->size(), z->size(), w->size()),
                                                                        m_Range(range),
                                                                        mp_TimeZone(timeZone),
                                                                        m_Matrix(m_Size.volume()),
                                                                        m_X(x),
                                                                        m_Y(y),
                                                                        m_Z(z),
                                                                        m_W(w) { }

        State(const Time::Range& range, const Axes::Axis<X>* x, const Axes::Axis<Y>* y,
              const Axes::Axis<Z>* z, const Axes::Axis<W>* w) : State(range, nullptr, x, y, z, w) {}

        State(const State &other) : m_Size(other.m_Size),
                                    m_Range(other.m_Range),
                                    mp_TimeZone(other.mp_TimeZone),
                                    m_Matrix(other.m_Matrix),
                                    m_X(other.m_X),
                                    m_Y(other.m_Y),
                                    m_Z(other.m_Z),
                                    m_W(other.m_W) { }

        State &operator=(const State &other) = default;

        ~State() = default;

        void printSize() const {
            std::cout << m_Size.width << 'x' << m_Size.height << 'x' << m_Size.depth << 'x' << m_Size.concepts <<
                std::endl;
        }

        void printFlatSize() const { std::cout << flatSize() << std::endl; }

        [[nodiscard]] bool isValid() const { return m_Size.isValid(); }

        [[nodiscard]] const Time::Range& range() const { return m_Range; }
        [[nodiscard]] const std::chrono::time_zone *timeZone() const { return mp_TimeZone; }

        [[nodiscard]] const Size& size() const { return m_Size; }

        [[nodiscard]] axis_size_t sizeX() const { return m_Size.width; }
        [[nodiscard]] axis_size_t sizeY() const { return m_Size.height; }
        [[nodiscard]] axis_size_t sizeZ() const { return m_Size.depth; }
        [[nodiscard]] axis_size_t sizeW() const { return m_Size.concepts; }

        [[nodiscard]] state_size_t flatSize() const { return m_Matrix.size(); }

        [[nodiscard]] const Axes::Axis<X>& x() const { return *m_X; }
        [[nodiscard]] const Axes::Axis<Y>& y() const { return *m_Y; }
        [[nodiscard]] const Axes::Axis<Z>& z() const { return *m_Z; }
        [[nodiscard]] const Axes::Axis<W>& w() const { return *m_W; }

        [[nodiscard]] const BitArray::BitArray &getBitArray() const { return m_Matrix; }

        [[nodiscard]] const Axes::AxisEntity& x(const axis_size_t xIndex) const { return (*m_X)[xIndex]; }
        [[nodiscard]] const Axes::AxisEntity& y(const axis_size_t yIndex) const { return (*m_Y)[yIndex]; }
        [[nodiscard]] const Axes::AxisEntity& z(const axis_size_t zIndex) const { return (*m_Z)[zIndex]; }
        [[nodiscard]] const Axes::AxisEntity& w(const axis_size_t wIndex) const { return (*m_W)[wIndex]; }

        uint8_t toggle(const axis_size_t x, const axis_size_t y, const axis_size_t z, const axis_size_t w) {
            const uint8_t newValue = m_Matrix.get(index(x, y, z, w) ^ 1) & 1;
            m_Matrix.assign(index(x, y, z, w), newValue);
            return newValue;
        }

        void assign(const axis_size_t x, const axis_size_t y, const axis_size_t z, const axis_size_t w,
                    const bool value) { m_Matrix.assign(index(x, y, z, w), static_cast<uint8_t>(value)); }

        void assign(const axis_size_t x, const axis_size_t y, const axis_size_t z, const axis_size_t w,
                    const uint8_t value) { m_Matrix.assign(index(x, y, z, w), value & 1); }

        void assign(const axis_size_t x, const axis_size_t y, const axis_size_t z, const axis_size_t w,
                    const uint32_t value) { m_Matrix.assign(index(x, y, z, w), value & 1); }

        void assign(const axis_size_t x, const axis_size_t y, const axis_size_t z, const axis_size_t w,
                    const int32_t value) { m_Matrix.assign(index(x, y, z, w), value & 1); }

        void set(const axis_size_t x, const axis_size_t y, const axis_size_t z, const axis_size_t w) {
            m_Matrix.set(index(x, y, z, w));
        }

        void clear(const axis_size_t x, const axis_size_t y, const axis_size_t z, const axis_size_t w) {
            m_Matrix.clear(index(x, y, z, w));
        }

        void setAll() {
            m_Matrix.setAll();
        }

        void clearAll() {
            m_Matrix.clearAll();
        }

        [[nodiscard]] uint8_t get(const axis_size_t x, const axis_size_t y, const axis_size_t z,
                                  const axis_size_t w) const { return m_Matrix.get(index(x, y, z, w)); }

        [[nodiscard]] uint8_t get(const axis_size_t x, const axis_size_t y, const axis_size_t z) const {
            return m_Matrix.test(offset(x, y, z), m_Size.concepts);
        }

        [[nodiscard]] uint8_t getXZ(const axis_size_t x, const axis_size_t z) const {
            for (axis_size_t y = 0; y < m_Size.height; ++y) {
                if (m_Matrix.test(offset(x, y, z), m_Size.concepts)) return 1;
            }
            return 0;
        }

        char getCharRepresentation(const axis_size_t x, const axis_size_t y, // NOLINT(*-use-nodiscard)
                                   const axis_size_t z, const axis_size_t w) const {
            return BitArray::BIT_CHAR_REPRESENTATIONS[get(x, y, z, w)];
        }

        void getLineXYW(BitArray::BitArray& dst, const axis_size_t x, const axis_size_t y,
                        const axis_size_t w) const {
            m_Matrix.copyTo(
                dst, x * m_Size.height * m_Size.depth * m_Size.concepts + y * m_Size.depth * m_Size.concepts + w,
                m_Size.concepts, 0);
        }

        void getLineXZW(BitArray::BitArray& dst, const axis_size_t x, const axis_size_t z,
                        const axis_size_t w) const {
            m_Matrix.copyTo(dst, x * m_Size.height * m_Size.depth * m_Size.concepts + z * m_Size.concepts + w,
                            m_Size.depth * m_Size.concepts, 0);
        }

        void getLineYZW(BitArray::BitArray& dst, const axis_size_t y, const axis_size_t z,
                        const axis_size_t w) const {
            m_Matrix.copyTo(dst, y * m_Size.depth * m_Size.concepts + z * m_Size.concepts + w,
                            m_Size.height * m_Size.depth * m_Size.concepts, 0);
        }

        void getLineXYZ(BitArray::BitArray& dst, const axis_size_t x, const axis_size_t y,
                        const axis_size_t z) const {
            m_Matrix.copyTo(
                dst,
                x * m_Size.height * m_Size.depth * m_Size.concepts + y * m_Size.depth * m_Size.concepts + z * m_Size.
                concepts, 1, 0);
        }

        void getPlaneXY(BitArray::BitArray& dst, const axis_size_t z, const axis_size_t w) const {
            for (axis_size_t x = 0; x < m_Size.width; ++x) {
                for (axis_size_t y = 0; y < m_Size.height; ++y) {
                    const BitArray::array_size_t srcIndex = index(x, y, z, w);
                    const BitArray::array_size_t dstIndex = x * m_Size.height + y;
                    dst.assign(dstIndex, m_Matrix.get(srcIndex));
                }
            }
        }

        void getPlaneXZ(BitArray::BitArray& dst, const axis_size_t y, const axis_size_t w) const {
            for (axis_size_t x = 0; x < m_Size.width; ++x) {
                for (axis_size_t z = 0; z < m_Size.depth; ++z) {
                    const BitArray::array_size_t srcIndex = index(x, y, z, w);
                    const BitArray::array_size_t dstIndex = x * m_Size.depth + z;
                    dst.assign(dstIndex, m_Matrix.get(srcIndex));
                }
            }
        }

        void getPlaneYZ(BitArray::BitArray& dst, const axis_size_t x, const axis_size_t w) const {
            for (axis_size_t y = 0; y < m_Size.height; ++y) {
                for (axis_size_t z = 0; z < m_Size.depth; ++z) {
                    const BitArray::array_size_t srcIndex = index(x, y, z, w);
                    const BitArray::array_size_t dstIndex = y * m_Size.depth + z;
                    dst.assign(dstIndex, m_Matrix.get(srcIndex));
                }
            }
        }

        void getPlaneXW(BitArray::BitArray& dst, const axis_size_t y, const axis_size_t z) const {
            for (axis_size_t x = 0; x < m_Size.width; ++x) {
                for (axis_size_t w = 0; w < m_Size.concepts; ++w) {
                    const BitArray::array_size_t srcIndex = index(x, y, z, w);
                    const BitArray::array_size_t dstIndex = x * m_Size.concepts + w;
                    dst.assign(dstIndex, m_Matrix.get(srcIndex));
                }
            }
        }

        void getPlaneYW(BitArray::BitArray& dst, const axis_size_t x, const axis_size_t z) const {
            for (axis_size_t y = 0; y < m_Size.height; ++y) {
                for (axis_size_t w = 0; w < m_Size.concepts; ++w) {
                    const BitArray::array_size_t srcIndex = index(x, y, z, w);
                    const BitArray::array_size_t dstIndex = y * m_Size.concepts + w;
                    dst.assign(dstIndex, m_Matrix.get(srcIndex));
                }
            }
        }

        void assignPlaneYW(const BitArray::BitArray& src, const axis_size_t x, const axis_size_t z) {
            for (axis_size_t y = 0; y < m_Size.height; ++y) {
                for (axis_size_t w = 0; w < m_Size.concepts; ++w) {
                    const BitArray::array_size_t dstIndex = index(x, y, z, w);
                    const BitArray::array_size_t srcIndex = y * m_Size.concepts + w;
                    m_Matrix.assign(dstIndex, src.get(srcIndex));
                }
            }
        }

        void clearPlaneYW(const axis_size_t x, const axis_size_t z) {
            for (axis_size_t y = 0; y < m_Size.height; ++y) {
                for (axis_size_t w = 0; w < m_Size.concepts; ++w) {
                    const BitArray::array_size_t dstIndex = index(x, y, z, w);
                    m_Matrix.clear(dstIndex);
                }
            }
        }

        void getPlaneZW(BitArray::BitArray& dst, const axis_size_t x, const axis_size_t y) const {
            for (axis_size_t z = 0; z < m_Size.depth; ++z) {
                for (axis_size_t w = 0; w < m_Size.concepts; ++w) {
                    const BitArray::array_size_t srcIndex = index(x, y, z, w);
                    const BitArray::array_size_t dstIndex = z * m_Size.concepts + w;
                    dst.assign(dstIndex, m_Matrix.get(srcIndex));
                }
            }
        }

        void collectTestIndicesW(const BitArray::BitArray& other, const axis_size_t x, const axis_size_t y,
                                 const axis_size_t z,
                                 std::vector<BitArray::array_size_t>& result) const {
            m_Matrix.collectTestIndices(other, offset(x, y, z), result);
        }

        void random(const float probability) { m_Matrix.random(probability); }

        void random() { m_Matrix.random(); }

    protected:
        Size m_Size;
        Time::Range m_Range;
        const std::chrono::time_zone *mp_TimeZone;
        BitArray::BitArray m_Matrix;

        [[nodiscard]] constexpr state_size_t offset(const axis_size_t x, const axis_size_t y) const {
            return m_Size.offset(x, y);
        }

        [[nodiscard]] constexpr state_size_t
        offset(const axis_size_t x, const axis_size_t y, const axis_size_t z) const {
            return m_Size.offset(x, y, z);
        }

        [[nodiscard]] constexpr state_size_t index(const axis_size_t x, const axis_size_t y, const axis_size_t z,
                                                   const axis_size_t w) const {
            return m_Size.index(x, y, z, w);
        }

    private:
        const Axes::Axis<X> *m_X;
        const Axes::Axis<Y> *m_Y;
        const Axes::Axis<Z> *m_Z;
        const Axes::Axis<W> *m_W;
    };
}

#endif //STATE_H

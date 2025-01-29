#ifndef STATE_H
#define STATE_H

#include <cassert>
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
        State(const Time::Range& range, const Axes::Axis<X>* x, const Axes::Axis<Y>* y,
              const Axes::Axis<Z>* z, const Axes::Axis<W>* w) : m_Size(x->size(), y->size(), z->size(), w->size()),
                                                                m_Range(range),
                                                                m_Matrix(m_Size.volume()),
                                                                m_X(x),
                                                                m_Y(y),
                                                                m_Z(z),
                                                                m_W(w) { }

        State(const State &other) : m_Size(other.m_Size),
                                    m_Range(other.m_Range),
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

        [[nodiscard]] const Time::Range& range() const { return m_Range; }

        [[nodiscard]] const Size& size() const { return m_Size; }

        [[nodiscard]] axis_size_t sizeX() const { return m_Size.width; }
        [[nodiscard]] axis_size_t sizeY() const { return m_Size.height; }
        [[nodiscard]] axis_size_t sizeZ() const { return m_Size.depth; }
        [[nodiscard]] axis_size_t sizeW() const { return m_Size.concepts; }

        [[nodiscard]] axis_size_t sizeWidth() const { return m_Size.width; }
        [[nodiscard]] axis_size_t sizeHeight() const { return m_Size.height; }
        [[nodiscard]] axis_size_t sizeDepth() const { return m_Size.depth; }
        [[nodiscard]] axis_size_t sizeConcepts() const { return m_Size.concepts; }

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

        [[nodiscard]] state_size_t offsetX(const axis_size_t y, const axis_size_t z, const axis_size_t w) const {
            return y * m_Size.depth * m_Size.concepts + z * m_Size.concepts + w;
        }

        [[nodiscard]] state_size_t offsetY(const axis_size_t x, const axis_size_t z, const axis_size_t w) const {
            return x * m_Size.height * m_Size.depth * m_Size.concepts + z * m_Size.concepts + w;
        }

        [[nodiscard]] state_size_t offsetZ(const axis_size_t x, const axis_size_t y, const axis_size_t w) const {
            return x * m_Size.height * m_Size.depth * m_Size.concepts + y * m_Size.depth * m_Size.concepts + w;
        }

        [[nodiscard]] state_size_t offsetW(const axis_size_t x, const axis_size_t y, const axis_size_t z) const {
            return x * m_Size.height * m_Size.depth * m_Size.concepts + y * m_Size.depth * m_Size.concepts + z * m_Size.
                concepts;
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
            m_Matrix.set(index(x, y, z, w));
        }

        [[nodiscard]] uint8_t get(const axis_size_t x, const axis_size_t y, const axis_size_t z,
                                  const axis_size_t w) const { return m_Matrix.get(offset(x, y, z, w)); }

        [[nodiscard]] uint8_t get(const axis_size_t x, const axis_size_t y, const axis_size_t z) const {
            return m_Matrix.test(offset(x, y, z), m_Size.concepts);
        }

        char getCharRepresentation(const axis_size_t x, const axis_size_t y, // NOLINT(*-use-nodiscard)
                                   const axis_size_t z, const axis_size_t w) const {
            return BitArray::BIT_CHAR_REPRESENTATIONS[get(x, y, z, w)];
        }

        void getLineXYW(BitArray::BitArray& array, const axis_size_t x, const axis_size_t y,
                        const axis_size_t w) const {
            m_Matrix.copyTo(
                array, x * m_Size.height * m_Size.depth * m_Size.concepts + y * m_Size.depth * m_Size.concepts + w,
                m_Size.concepts, 0);
        }

        void getLineXZW(BitArray::BitArray& array, const axis_size_t x, const axis_size_t z,
                        const axis_size_t w) const {
            m_Matrix.copyTo(array, x * m_Size.height * m_Size.depth * m_Size.concepts + z * m_Size.concepts + w,
                            m_Size.depth * m_Size.concepts, 0);
        }

        void getLineYZW(BitArray::BitArray& array, const axis_size_t y, const axis_size_t z,
                        const axis_size_t w) const {
            m_Matrix.copyTo(array, y * m_Size.depth * m_Size.concepts + z * m_Size.concepts + w,
                            m_Size.height * m_Size.depth * m_Size.concepts, 0);
        }

        void getLineXYZ(BitArray::BitArray& array, const axis_size_t x, const axis_size_t y,
                        const axis_size_t z) const {
            m_Matrix.copyTo(
                array,
                x * m_Size.height * m_Size.depth * m_Size.concepts + y * m_Size.depth * m_Size.concepts + z * m_Size.
                concepts, 1, 0);
        }

        void getPlaneXY(BitArray::BitArray& array, const axis_size_t z, const axis_size_t w) const {
            m_Matrix.copyTo(array, z * m_Size.concepts + w, m_Size.depth * m_Size.concepts, 0);
        }

        void getPlaneXZ(BitArray::BitArray& array, const axis_size_t y, const axis_size_t w) const {
            m_Matrix.copyTo(array, y * m_Size.depth * m_Size.concepts + w, m_Size.height * m_Size.concepts, 0);
        }

        void getPlaneYZ(BitArray::BitArray& array, const axis_size_t x, const axis_size_t w) const {
            m_Matrix.copyTo(array, x * m_Size.height * m_Size.depth * m_Size.concepts + w,
                            m_Size.width * m_Size.concepts, 0);
        }

        void getPlaneXW(BitArray::BitArray& array, const axis_size_t y, const axis_size_t z) const {
            m_Matrix.copyTo(array, y * m_Size.depth * m_Size.concepts + z * m_Size.concepts,
                            m_Size.height * m_Size.depth, 0);
        }

        void getPlaneYW(BitArray::BitArray& array, const axis_size_t x, const axis_size_t z) const {
            m_Matrix.copyTo(array, x * m_Size.height * m_Size.depth * m_Size.concepts + z * m_Size.concepts,
                            m_Size.width * m_Size.depth, 0);
        }

        void getPlaneZW(BitArray::BitArray& array, const axis_size_t x, const axis_size_t y) const {
            m_Matrix.copyTo(
                array, x * m_Size.height * m_Size.depth * m_Size.concepts + y * m_Size.depth * m_Size.concepts,
                m_Size.width * m_Size.height, 0);
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
        BitArray::BitArray m_Matrix;

        [[nodiscard]] constexpr state_size_t offset(const axis_size_t x, const axis_size_t y) const {
            assert(x < m_Size.width && "X must be less than the width.");
            assert(y < m_Size.height && "Y must be less than the height.");
            return x * m_Size.height * m_Size.depth * m_Size.concepts + y * m_Size.depth * m_Size.concepts;
        }

        [[nodiscard]] constexpr state_size_t
        offset(const axis_size_t x, const axis_size_t y, const axis_size_t z) const {
            assert(z < m_Size.depth && "Z must be less than the depth.");
            return offset(x, y) + z * m_Size.concepts;
        }

        [[nodiscard]] constexpr state_size_t index(const axis_size_t x, const axis_size_t y, const axis_size_t z,
                                                   const axis_size_t w) const {
            assert(w < m_Size.concepts && "W must be less than the total concept count.");
            return offset(x, y, z) + w;
        }

    private:
        const Axes::Axis<X> *m_X;
        const Axes::Axis<Y> *m_Y;
        const Axes::Axis<Z> *m_Z;
        const Axes::Axis<W> *m_W;
    };
}

#endif //STATE_H

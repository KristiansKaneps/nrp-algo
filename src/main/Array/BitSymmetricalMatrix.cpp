#include "Array/BitSymmetricalMatrix.h"

namespace BitMatrix {
    BitSymmetricalMatrix createSymmetricalMatrix(const array_size_t dimensionSize) {
        BitSymmetricalMatrix matrix(dimensionSize);
        return matrix;
    }

    BitSymmetricalMatrix createIdentitySymmetricalMatrix(const array_size_t dimensionSize) {
        BitSymmetricalMatrix matrix(dimensionSize);
        for (array_size_t x = 0; x < dimensionSize; ++x)
            matrix.set(x, x);
        return matrix;
    }
}

#include "Array/BitSquareMatrix.h"

namespace BitMatrix {
    BitSquareMatrix createSquareMatrix(const array_size_t dimensionSize) {
        BitSquareMatrix matrix(dimensionSize);
        return matrix;
    }

    BitSquareMatrix createIdentitySquareMatrix(const array_size_t dimensionSize) {
        BitSquareMatrix matrix(dimensionSize);
        for (array_size_t x = 0; x < dimensionSize; ++x)
            matrix.set(x, x);
        return matrix;
    }
}

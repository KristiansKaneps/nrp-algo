#include "Array/BitMatrix.h"

namespace BitMatrix {
    BitMatrix createMatrix(const array_size_t xSize, const array_size_t ySize) {
        BitMatrix matrix(xSize, ySize);
        return matrix;
    }

    BitMatrix3D createMatrix3D(const array_size_t xSize, const array_size_t ySize, const array_size_t zSize) {
        BitMatrix3D matrix(xSize, ySize, zSize);
        return matrix;
    }
}

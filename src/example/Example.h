#ifndef EXAMPLE_H
#define EXAMPLE_H

// #define EXAMPLE 1
// #define EXAMPLE 2
// #define EXAMPLE 3
#define EXAMPLE 4

#include <iostream>
#include "ApplicationState.h"

namespace Example {
    struct Options {
        std::string_view arg;
    };

    void create(const Options& options);
}

#endif //EXAMPLE_H

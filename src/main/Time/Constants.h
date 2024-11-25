#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <chrono>

namespace Time {
    // ReSharper disable CppRedundantTemplateArguments
    using Instant = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;

    constexpr Instant MIN_INSTANT = Instant::min();
    constexpr Instant MAX_INSTANT = Instant::max();
}

#endif //CONSTANTS_H

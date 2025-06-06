#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <ctime>

namespace String {
    inline std::tm localtime_xp(const std::time_t timer) {
        std::tm bt {};
        #if defined(__unix__)
        localtime_r(&timer, &bt);
        #elif defined(_MSC_VER)
        localtime_s(&bt, &timer);
        #else
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        bt = *std::localtime(&timer);
        #endif
        return bt;
    }
}

#endif //STRINGUTILS_H

#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <ctime>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace String {
    inline std::tm localtime_xp(const std::time_t timer) noexcept {
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

    inline std::string getTimestampPrefix() noexcept {
        const auto now = std::chrono::system_clock::now();
        const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        const std::tm tm = String::localtime_xp(now_time_t);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S_"); // e.g., "2025-06-06_13-42-05_"
        return oss.str();
    }
}

#endif //STRINGUTILS_H

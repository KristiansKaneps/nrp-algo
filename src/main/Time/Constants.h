#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <iostream>
#include <chrono>
#include <regex>

#define INSTANT_PRECISION std::chrono::milliseconds

namespace Time {
    // ReSharper disable CppRedundantTemplateArguments
    using Instant = std::chrono::time_point<std::chrono::system_clock, INSTANT_PRECISION>;

    constexpr Instant MIN_INSTANT = Instant::min();
    constexpr Instant MAX_INSTANT = Instant::max();

    template<typename Duration = std::chrono::minutes>
    static std::string InstantToString(const std::chrono::time_point<std::chrono::system_clock, Duration>& instant) {
        // Truncate to seconds for base time formatting
        const auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(instant);
        // Extract nanoseconds as a remainder
        const auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(instant.time_since_epoch()) %
            1'000'000'000;
        // Format the base time and append nanoseconds
        return std::format("{:%FT%T}.{:09d}Z", seconds, nanoseconds.count());
    }

    static Instant StringToInstant(const std::string& input) {
        // Regular expression to match the supported formats:
        // 1. ISO 8601 with Z: "2024-12-17T18:00:00.123456789Z"
        // 2. ISO 8601 with space and offset: "2024-12-17 18:00:00+00"
        static const std::regex regex(
            R"((\d{4})-(\d{2})-(\d{2})[T ](\d{2}):(\d{2}):(\d{2})(?:\.(\d+))?([+-]\d{2}:?\d{2}|[+-]\d{2}|Z)?)");
        std::smatch match;

        if (!std::regex_match(input, match, regex)) [[unlikely]] {
            std::cerr << "std::regex_match failed for input: " << input << std::endl;
            throw std::invalid_argument("Invalid date-time format");
        }

        // Extract the main components
        const int year = std::stoi(match[1].str());
        const unsigned int month = std::stoi(match[2].str());
        const unsigned int day = std::stoi(match[3].str());
        const unsigned int hour = std::stoi(match[4].str());
        const unsigned int minute = std::stoi(match[5].str());
        const unsigned int second = std::stoi(match[6].str());

        // Extract fractional seconds if they exist
        int64_t nanoseconds = 0;
        if (match[7].matched) {
            std::string fraction = match[7].str();
            // Normalize the fractional seconds to nanoseconds
            if (fraction.size() < 9) {
                fraction.append(9 - fraction.size(), '0'); // Pad with zeros to nanoseconds
            } else if (fraction.size() > 9) {
                fraction = fraction.substr(0, 9); // Truncate to nanoseconds
            }
            nanoseconds = std::stoll(fraction);
        }

        // Extract the UTC offset (e.g., "+00", "+02:00", "Z")
        const std::string offset = match[8].str();
        std::chrono::duration<int> offsetDuration;
        if (offset == "Z" || offset == "+00" || offset == "+00:00") {
            offsetDuration = std::chrono::hours(0); // UTC
        } else {
            // Parse offset: "+HH:MM" or "+HHMM"
            const int sign = (offset[0] == '+') ? 1 : -1;
            const int hours = std::stoi(offset.substr(1, 2));
            const int minutes = (offset.size() == 5 || offset.size() == 6) ? std::stoi(offset.substr(4, 2)) : 0;
            offsetDuration = std::chrono::hours(sign * hours) + std::chrono::minutes(sign * minutes);
        }

        // Create the time point using UTC, then adjust by the offset.
        const std::chrono::year_month_day ymd{std::chrono::year{year}, std::chrono::month{month}, std::chrono::day{day}};
        const auto sec = std::chrono::seconds{std::chrono::hours{hour} + std::chrono::minutes{minute} + std::chrono::seconds{second}};

        // Use system_clock to get the UTC time and then adjust for the offset.
        auto timePoint = std::chrono::sys_days{ymd} + sec;
        timePoint -= offsetDuration;

        // Return the time_point with nanoseconds precision
        return std::chrono::time_point_cast<INSTANT_PRECISION>(timePoint + std::chrono::nanoseconds(nanoseconds));
    }

    /**
     * @return weekday as an unsigned 8-bit int (0 - Monday, ..., 6 - Sunday)
     */
    template<typename TimeZone = const std::chrono::time_zone *>
    static uint8_t InstantToWeekday(const Instant& instant, TimeZone zone) {
        auto localTime = zone->to_local(instant);
        // POSIX standard (0 - Sunday, 1 - Monday, ..., 6 - Saturday).
        const auto weekdayIsoStd = std::chrono::year_month_weekday{std::chrono::floor<std::chrono::days>(localTime)}.weekday();
        // `is_encoding()` converts POSIX to ISO 8601 standard (1 - Monday, ..., 7 - Sunday)
        return static_cast<uint8_t>(weekdayIsoStd.iso_encoding() - 1);
    }

    /**
     * @return weekday as an unsigned 8-bit int (0 - Monday, ..., 6 - Sunday)
     */
    template<typename TimeZone = const std::chrono::time_zone *>
    static uint8_t InstantToWeekday(const std::chrono::zoned_time<TimeZone>& zonedInstant) {
        auto localTime = zonedInstant.get_local_time();
        // POSIX standard (0 - Sunday, 1 - Monday, ..., 6 - Saturday).
        const auto weekdayIsoStd = std::chrono::year_month_weekday{std::chrono::floor<std::chrono::days>(localTime)}.weekday();
        // `is_encoding()` converts POSIX to ISO 8601 standard (1 - Monday, ..., 7 - Sunday)
        return static_cast<uint8_t>(weekdayIsoStd.iso_encoding() - 1);
    }

    template<typename Duration = std::chrono::minutes>
    std::ostream& operator<<(std::ostream& out, const std::chrono::time_point<std::chrono::system_clock, Duration>& instant) {
        out << InstantToString(instant);
        return out;
    }
}

#endif //CONSTANTS_H

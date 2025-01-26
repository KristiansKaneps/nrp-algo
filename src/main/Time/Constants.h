#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <iostream>
#include <chrono>
#include <regex>

namespace Time {
    // ReSharper disable CppRedundantTemplateArguments
    using Instant = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;

    constexpr Instant MIN_INSTANT = Instant::min();
    constexpr Instant MAX_INSTANT = Instant::max();

    static std::string InstantToString(const Instant& instant) {
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
        const int month = std::stoi(match[2].str());
        const int day = std::stoi(match[3].str());
        const int hour = std::stoi(match[4].str());
        const int minute = std::stoi(match[5].str());
        const int second = std::stoi(match[6].str());

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
        int offsetSeconds = 0;
        if (match[8].matched) {
            if (const std::string offset = match[8].str(); offset == "Z" || offset == "+00" || offset == "+00:00") {
                offsetSeconds = 0; // UTC
            } else {
                // Parse offset: "+HH:MM" or "+HHMM"
                const int sign = (offset[0] == '+') ? 1 : -1;
                const int hours = std::stoi(offset.substr(1, 2));
                const int minutes = (offset.size() == 5 || offset.size() == 6) ? std::stoi(offset.substr(4, 2)) : 0;
                offsetSeconds = sign * (hours * 3600 + minutes * 60);
            }
        }

        // Create a tm struct for the base time
        std::tm tm = {};
        tm.tm_year = year - 1900; // tm_year is years since 1900
        tm.tm_mon = month - 1;    // tm_mon is 0-based
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = second;

        // Convert tm to time_t
        const std::time_t timeT = std::mktime(&tm); // This assumes local time
        if (timeT == -1) [[unlikely]] {
            std::cerr << "std::mktime failed for input: " << input << std::endl;
            throw std::runtime_error("Failed to convert time");
        }

        // Convert to UTC by subtracting the offset
        const std::time_t utcTimeT = timeT - offsetSeconds;

        // Build the time_point with nanoseconds precision
        const auto baseTime = std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>(
            std::chrono::seconds(utcTimeT));
        return baseTime + std::chrono::nanoseconds(nanoseconds);
    }
}

#endif //CONSTANTS_H

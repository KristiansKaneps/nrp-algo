#include "doctest.h"

#include <chrono>

#include "Time/Range.h"
#include "Time/DailyInterval.h"

SCENARIO("daily interval to range") {
    GIVEN("a bunch of intervals") {
        const auto interval1 = Time::DailyInterval("08:00", "16:00");
        const auto interval2 = Time::DailyInterval("16:00", "24:00");
        const auto interval3 = Time::DailyInterval("08:00", "32:00");
        const auto interval4 = Time::DailyInterval("16:00", "40:00");

        const auto instant = Time::StringToInstant("2025-02-02T00:00:00Z");

        const std::chrono::tzdb& tzdb = std::chrono::get_tzdb();
        const std::chrono::time_zone* utc = tzdb.locate_zone("UTC");

        const auto day = std::chrono::zoned_time(utc, instant);

        WHEN("expanding intervals to ranges") {
            const auto range1 = interval1.toRange(day);
            const auto range2 = interval2.toRange(day);
            const auto range3 = interval3.toRange(day);
            const auto range4 = interval4.toRange(day);

            using std::chrono_literals::operator ""min;

            THEN("their starting point must match") {
                const int16_t startMinutes1 = static_cast<int16_t>((range1.start() - std::chrono::floor<std::chrono::days>(range1.start())) / 1min);
                const int16_t startMinutes2 = static_cast<int16_t>((range2.start() - std::chrono::floor<std::chrono::days>(range2.start())) / 1min);
                const int16_t startMinutes3 = static_cast<int16_t>((range3.start() - std::chrono::floor<std::chrono::days>(range3.start())) / 1min);
                const int16_t startMinutes4 = static_cast<int16_t>((range4.start() - std::chrono::floor<std::chrono::days>(range4.start())) / 1min);
                CHECK(startMinutes1 == interval1.startInMinutes());
                CHECK(startMinutes2 == interval2.startInMinutes());
                CHECK(startMinutes3 == interval3.startInMinutes());
                CHECK(startMinutes4 == interval4.startInMinutes());
            }

            THEN("their duration must match") {
                const int16_t durationMinutes1 = static_cast<int16_t>(range1.duration() / 1min);
                const int16_t durationMinutes2 = static_cast<int16_t>(range2.duration() / 1min);
                const int16_t durationMinutes3 = static_cast<int16_t>(range3.duration() / 1min);
                const int16_t durationMinutes4 = static_cast<int16_t>(range4.duration() / 1min);
                CHECK(durationMinutes1 == interval1.durationInMinutes());
                CHECK(durationMinutes2 == interval2.durationInMinutes());
                CHECK(durationMinutes3 == interval3.durationInMinutes());
                CHECK(durationMinutes4 == interval4.durationInMinutes());
            }
        }
    }

    GIVEN("a bunch of intervals (some crossing DST, one hour forward)") {
        const auto interval1 = Time::DailyInterval("08:00", "16:00");
        const auto interval2 = Time::DailyInterval("16:00", "24:00");
        const auto interval3 = Time::DailyInterval("08:00", "32:00");
        const auto interval4 = Time::DailyInterval("16:00", "40:00");

        const auto instant = Time::StringToInstant("2025-03-29T00:00:00+02:00");

        const std::chrono::tzdb& tzdb = std::chrono::get_tzdb();
        const std::chrono::time_zone* zone = tzdb.locate_zone("Europe/Riga");

        const auto day = std::chrono::zoned_time(zone, instant);

        WHEN("expanding intervals to ranges") {
            const auto range1 = interval1.toRange(day);
            const auto range2 = interval2.toRange(day);
            const auto range3 = interval3.toRange(day);
            const auto range4 = interval4.toRange(day);

            using std::chrono_literals::operator ""min;

            THEN("their starting point must match") {
                const auto localStart1 = std::chrono::floor<std::chrono::days>(std::chrono::zoned_time(zone, range1.start()).get_local_time());
                const auto localStart2 = std::chrono::floor<std::chrono::days>(std::chrono::zoned_time(zone, range2.start()).get_local_time());
                const auto localStart3 = std::chrono::floor<std::chrono::days>(std::chrono::zoned_time(zone, range3.start()).get_local_time());
                const auto localStart4 = std::chrono::floor<std::chrono::days>(std::chrono::zoned_time(zone, range4.start()).get_local_time());
                const int16_t startMinutes1 = static_cast<int16_t>((range1.start() - zone->to_sys(localStart1)) / 1min);
                const int16_t startMinutes2 = static_cast<int16_t>((range2.start() - zone->to_sys(localStart2)) / 1min);
                const int16_t startMinutes3 = static_cast<int16_t>((range3.start() - zone->to_sys(localStart3)) / 1min);
                const int16_t startMinutes4 = static_cast<int16_t>((range4.start() - zone->to_sys(localStart4)) / 1min);
                CHECK(startMinutes1 == interval1.startInMinutes());
                CHECK(startMinutes2 == interval2.startInMinutes());
                CHECK(startMinutes3 == interval3.startInMinutes());
                CHECK(startMinutes4 == interval4.startInMinutes());
            }

            THEN("their duration must match (according to DST)") {
                const int16_t durationMinutes1 = static_cast<int16_t>(range1.duration() / 1min);
                const int16_t durationMinutes2 = static_cast<int16_t>(range2.duration() / 1min);
                const int16_t durationMinutes3 = static_cast<int16_t>(range3.duration() / 1min);
                const int16_t durationMinutes4 = static_cast<int16_t>(range4.duration() / 1min);
                CHECK(durationMinutes1 == interval1.durationInMinutes());
                CHECK(durationMinutes2 == interval2.durationInMinutes());
                CHECK(durationMinutes3 + 60 == interval3.durationInMinutes());
                CHECK(durationMinutes4 + 60 == interval4.durationInMinutes());
            }
        }
    }

    GIVEN("a bunch of intervals (some crossing DST, one hour backward)") {
        const auto interval1 = Time::DailyInterval("08:00", "16:00");
        const auto interval2 = Time::DailyInterval("16:00", "24:00");
        const auto interval3 = Time::DailyInterval("08:00", "32:00");
        const auto interval4 = Time::DailyInterval("16:00", "40:00");

        const auto instant = Time::StringToInstant("2024-10-26T00:00:00+03:00");

        const std::chrono::tzdb& tzdb = std::chrono::get_tzdb();
        const std::chrono::time_zone* zone = tzdb.locate_zone("Europe/Riga");

        const auto day = std::chrono::zoned_time(zone, instant);

        WHEN("expanding intervals to ranges") {
            const auto range1 = interval1.toRange(day);
            const auto range2 = interval2.toRange(day);
            const auto range3 = interval3.toRange(day);
            const auto range4 = interval4.toRange(day);

            using std::chrono_literals::operator ""min;

            THEN("their starting point must match") {
                const auto localStart1 = std::chrono::floor<std::chrono::days>(std::chrono::zoned_time(zone, range1.start()).get_local_time());
                const auto localStart2 = std::chrono::floor<std::chrono::days>(std::chrono::zoned_time(zone, range2.start()).get_local_time());
                const auto localStart3 = std::chrono::floor<std::chrono::days>(std::chrono::zoned_time(zone, range3.start()).get_local_time());
                const auto localStart4 = std::chrono::floor<std::chrono::days>(std::chrono::zoned_time(zone, range4.start()).get_local_time());
                const int16_t startMinutes1 = static_cast<int16_t>((range1.start() - zone->to_sys(localStart1)) / 1min);
                const int16_t startMinutes2 = static_cast<int16_t>((range2.start() - zone->to_sys(localStart2)) / 1min);
                const int16_t startMinutes3 = static_cast<int16_t>((range3.start() - zone->to_sys(localStart3)) / 1min);
                const int16_t startMinutes4 = static_cast<int16_t>((range4.start() - zone->to_sys(localStart4)) / 1min);
                CHECK(startMinutes1 == interval1.startInMinutes());
                CHECK(startMinutes2 == interval2.startInMinutes());
                CHECK(startMinutes3 == interval3.startInMinutes());
                CHECK(startMinutes4 == interval4.startInMinutes());
            }

            THEN("their duration must match (according to DST)") {
                const int16_t durationMinutes1 = static_cast<int16_t>(range1.duration() / 1min);
                const int16_t durationMinutes2 = static_cast<int16_t>(range2.duration() / 1min);
                const int16_t durationMinutes3 = static_cast<int16_t>(range3.duration() / 1min);
                const int16_t durationMinutes4 = static_cast<int16_t>(range4.duration() / 1min);
                CHECK(durationMinutes1 == interval1.durationInMinutes());
                CHECK(durationMinutes2 == interval2.durationInMinutes());
                CHECK(durationMinutes3 - 60 == interval3.durationInMinutes());
                CHECK(durationMinutes4 - 60 == interval4.durationInMinutes());
            }
        }
    }
}
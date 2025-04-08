#include "doctest.h"

#include <chrono>

#include "Time/Range.h"
#include "Time/DailyInterval.h"

SCENARIO("daily interval intersection and conversion to range") {
    GIVEN("a bunch of intervals") {
        const auto interval1 = Time::DailyInterval("08:00", "16:00");
        const auto interval2 = Time::DailyInterval("16:00", "24:00");
        const auto interval3 = Time::DailyInterval("08:00", "32:00");
        const auto interval4 = Time::DailyInterval("16:00", "40:00");

        const auto instant = Time::StringToInstant("2025-02-02T00:00:00Z");

        const std::chrono::tzdb& tzdb = std::chrono::get_tzdb();
        const std::chrono::time_zone* utc = tzdb.locate_zone("UTC");

        const auto day = std::chrono::zoned_time(utc, instant);

        WHEN("checking interval intersections in the same day") {
            THEN("first and second interval should not intersect in the same day") {
                CHECK(!interval1.intersectsInSameDay(interval2));
                CHECK(!interval2.intersectsInSameDay(interval1));
                CHECK(!interval1.intersectsOtherInOffsetDay(interval2, 0));
                CHECK(!interval2.intersectsOtherInOffsetDay(interval1, 0));
            }

            THEN("second and third interval should intersect in the same day") {
                CHECK(interval2.intersectsInSameDay(interval3));
                CHECK(interval3.intersectsInSameDay(interval2));
                CHECK(interval2.intersectsOtherInOffsetDay(interval3, 0));
                CHECK(interval3.intersectsOtherInOffsetDay(interval2, 0));
            }

            THEN("third and fourth interval should intersect in the same day") {
                CHECK(interval4.intersectsInSameDay(interval3));
                CHECK(interval3.intersectsInSameDay(interval4));
                CHECK(interval4.intersectsOtherInOffsetDay(interval3, 0));
                CHECK(interval3.intersectsOtherInOffsetDay(interval4, 0));
            }

            THEN("first and fourth interval should not intersect in the same day") {
                CHECK(!interval1.intersectsInSameDay(interval4));
                CHECK(!interval4.intersectsInSameDay(interval1));
                CHECK(!interval1.intersectsOtherInOffsetDay(interval4, 0));
                CHECK(!interval4.intersectsOtherInOffsetDay(interval1, 0));
            }

            THEN("second and fourth interval should intersect in the same day") {
                CHECK(interval2.intersectsInSameDay(interval4));
                CHECK(interval4.intersectsInSameDay(interval2));
                CHECK(interval2.intersectsOtherInOffsetDay(interval4, 0));
                CHECK(interval4.intersectsOtherInOffsetDay(interval2, 0));
            }

            THEN("third and fourth interval should intersect in the same day") {
                CHECK(interval3.intersectsInSameDay(interval4));
                CHECK(interval4.intersectsInSameDay(interval3));
                CHECK(interval3.intersectsOtherInOffsetDay(interval4, 0));
                CHECK(interval4.intersectsOtherInOffsetDay(interval3, 0));
            }

            THEN("second and third interval should intersect in the same day") {
                CHECK(interval2.intersectsInSameDay(interval3));
                CHECK(interval3.intersectsInSameDay(interval2));
                CHECK(interval2.intersectsOtherInOffsetDay(interval3, 0));
                CHECK(interval3.intersectsOtherInOffsetDay(interval2, 0));
            }
        }

        WHEN("checking interval intersections in two adjacent days") {
            THEN("first and second interval should not intersect (interval1 -> interval2 and interval2 -> interval1)") {
                CHECK(!interval1.intersectsOtherInNextDay(interval2));
                CHECK(!interval2.intersectsOtherInPrevDay(interval1));
                CHECK(!interval2.intersectsOtherInNextDay(interval1));
                CHECK(!interval1.intersectsOtherInPrevDay(interval2));
                CHECK(!interval1.intersectsOtherInOffsetDay(interval2, 1));
                CHECK(!interval2.intersectsOtherInOffsetDay(interval1, -1));
                CHECK(!interval2.intersectsOtherInOffsetDay(interval1, 1));
                CHECK(!interval1.intersectsOtherInOffsetDay(interval2, -1));
            }

            THEN("second and third interval should not intersect (interval2 -> interval3 and interval3 -> interval2)") {
                CHECK(!interval2.intersectsOtherInNextDay(interval3));
                CHECK(!interval3.intersectsOtherInPrevDay(interval2));
                CHECK(!interval3.intersectsOtherInNextDay(interval2));
                CHECK(!interval2.intersectsOtherInPrevDay(interval3));
                CHECK(!interval2.intersectsOtherInOffsetDay(interval3, 1));
                CHECK(!interval3.intersectsOtherInOffsetDay(interval2, -1));
                CHECK(!interval3.intersectsOtherInOffsetDay(interval2, 1));
                CHECK(!interval2.intersectsOtherInOffsetDay(interval3, -1));
            }

            THEN("third and fourth interval should not intersect (interval3 -> interval4)") {
                CHECK(!interval3.intersectsOtherInNextDay(interval4));
                CHECK(!interval4.intersectsOtherInPrevDay(interval3));
                CHECK(!interval3.intersectsOtherInOffsetDay(interval4, 1));
                CHECK(!interval4.intersectsOtherInOffsetDay(interval3, -1));
            }

            THEN("third and fourth interval should intersect (interval4 -> interval3)") {
                CHECK(interval4.intersectsOtherInNextDay(interval3));
                CHECK(interval3.intersectsOtherInPrevDay(interval4));
                CHECK(interval4.intersectsOtherInOffsetDay(interval3, 1));
                CHECK(interval3.intersectsOtherInOffsetDay(interval4, -1));
            }

            THEN("first and fourth interval should not intersect (interval1 -> interval4)") {
                CHECK(!interval1.intersectsOtherInNextDay(interval4));
                CHECK(!interval4.intersectsOtherInPrevDay(interval1));
                CHECK(!interval1.intersectsOtherInOffsetDay(interval4, 1));
                CHECK(!interval4.intersectsOtherInOffsetDay(interval1, -1));
            }

            THEN("first and fourth interval should intersect (interval4 -> interval1)") {
                CHECK(interval4.intersectsOtherInNextDay(interval1));
                CHECK(interval1.intersectsOtherInPrevDay(interval4));
                CHECK(interval4.intersectsOtherInOffsetDay(interval1, 1));
                CHECK(interval1.intersectsOtherInOffsetDay(interval4, -1));
            }

            THEN("second and fourth interval should not intersect (interval2 -> interval4 and interval4 -> interval2)") {
                CHECK(!interval2.intersectsOtherInNextDay(interval4));
                CHECK(!interval4.intersectsOtherInPrevDay(interval2));
                CHECK(!interval4.intersectsOtherInNextDay(interval2));
                CHECK(!interval2.intersectsOtherInPrevDay(interval4));
                CHECK(!interval2.intersectsOtherInOffsetDay(interval4, 1));
                CHECK(!interval4.intersectsOtherInOffsetDay(interval2, -1));
                CHECK(!interval4.intersectsOtherInOffsetDay(interval2, 1));
                CHECK(!interval2.intersectsOtherInOffsetDay(interval4, -1));
            }

            THEN("third and fourth interval should not intersect (interval3 -> interval4)") {
                CHECK(!interval3.intersectsOtherInNextDay(interval4));
                CHECK(!interval4.intersectsOtherInPrevDay(interval3));
                CHECK(!interval3.intersectsOtherInOffsetDay(interval4, 1));
                CHECK(!interval4.intersectsOtherInOffsetDay(interval3, -1));
            }

            THEN("third and fourth interval should intersect (interval4 -> interval3)") {
                CHECK(!interval4.intersectsOtherInNextDay(interval2));
                CHECK(!interval2.intersectsOtherInPrevDay(interval4));
                CHECK(!interval4.intersectsOtherInOffsetDay(interval2, 1));
                CHECK(!interval2.intersectsOtherInOffsetDay(interval4, -1));
            }

            THEN("second and third interval should not intersect (interval2 -> interval3 and interval3 -> interval2)") {
                CHECK(!interval2.intersectsOtherInNextDay(interval3));
                CHECK(!interval3.intersectsOtherInPrevDay(interval2));
                CHECK(!interval3.intersectsOtherInNextDay(interval2));
                CHECK(!interval2.intersectsOtherInPrevDay(interval3));
                CHECK(!interval2.intersectsOtherInOffsetDay(interval3, 1));
                CHECK(!interval3.intersectsOtherInOffsetDay(interval2, -1));
                CHECK(!interval3.intersectsOtherInOffsetDay(interval2, 1));
                CHECK(!interval2.intersectsOtherInOffsetDay(interval3, -1));
            }
        }

        WHEN("checking 24h interval intersections in two 'adjacent' days (padded with 48h and 2 day offset)") {
            const auto interval3Padded = interval3.withPadding(48 * 60);
            const auto interval4Padded = interval4.withPadding(48 * 60);

            THEN("there should be an intersection (interval3)") {
                CHECK(interval3Padded.intersectsOtherInOffsetDay(interval3, 2));
                CHECK(interval3.intersectsOtherInOffsetDay(interval3Padded, 2));
                CHECK(interval3Padded.intersectsOtherInOffsetDay(interval3, -2));
                CHECK(interval3.intersectsOtherInOffsetDay(interval3Padded, -2));
            }

            THEN("there should be an intersection (interval4)") {
                CHECK(interval4Padded.intersectsOtherInOffsetDay(interval4, 2));
                CHECK(interval4.intersectsOtherInOffsetDay(interval4Padded, 2));
                CHECK(interval4Padded.intersectsOtherInOffsetDay(interval4, -2));
                CHECK(interval4.intersectsOtherInOffsetDay(interval4Padded, -2));
            }
        }

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
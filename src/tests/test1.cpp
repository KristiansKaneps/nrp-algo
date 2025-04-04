#include "doctest.h"

#include <chrono>

#include "Time/Range.h"

SCENARIO("time range day intervals") {
    GIVEN("a month range") {
        const auto startUtc = Time::StringToInstant("2025-02-01T00:00:00Z");
        const auto endUtc = Time::StringToInstant("2025-03-01T00:00:00Z");
        const auto startLocal = Time::StringToInstant("2025-02-01T00:00:00+02:00");
        const auto endLocal = Time::StringToInstant("2025-03-01T00:00:00+02:00");

        const Time::Range rangeUtc(startUtc, endUtc);
        const Time::Range rangeLocal(startLocal, endLocal);

        const std::chrono::tzdb& tzdb = std::chrono::get_tzdb();
        const std::chrono::time_zone* utc = tzdb.locate_zone("UTC");

        WHEN("getting day range at starting instant (which is at 00:00 UTC)") {
            const auto dayRange = rangeUtc.getDayRangeAt(0, utc);

            THEN("its start point is expected to be the same as month range's") {
                CHECK(dayRange.start() == rangeUtc.start());
            }

            THEN("its duration is expected to be 24h") {
                using std::chrono_literals::operator ""h;
                const int hours = dayRange.duration() / 1h;
                CHECK(hours == 24);
            }

            THEN("its end point is expected to be after 24h") {
                using std::chrono_literals::operator ""h;
                const auto expectedEndPoint = dayRange.start() + 24h;
                CHECK(dayRange.end() == expectedEndPoint);
            }
        }

        WHEN("getting day range at starting instant (which is at 22:00 UTC)") {
            const auto dayRange = rangeLocal.getDayRangeAt(0, utc);

            THEN("its start point is expected to be the same as month range's") {
                CHECK(dayRange.start() == rangeLocal.start());
            }

            THEN("its duration is expected to be 2h") {
                using std::chrono_literals::operator ""h;
                const int hours = dayRange.duration() / 1h;
                CHECK(hours == 2);
            }

            THEN("its end point is expected to be after 2h") {
                using std::chrono_literals::operator ""h;
                const auto expectedEndPoint = dayRange.start() + 2h;
                CHECK(dayRange.end() == expectedEndPoint);
            }
        }
    }
}
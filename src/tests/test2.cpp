#include "doctest.h"

#include <chrono>

#include "Time/Range.h"

SCENARIO("time range day count") {
    GIVEN("a five workday range") {
        const auto startUtc = Time::StringToInstant("2025-02-03T00:00:00Z");
        const auto endUtc = Time::StringToInstant("2025-02-08T00:00:00Z");

        const Time::Range rangeUtc(startUtc, endUtc);

        const std::chrono::tzdb& tzdb = std::chrono::get_tzdb();
        const std::chrono::time_zone* utc = tzdb.locate_zone("UTC");

        WHEN("getting workday day count") {
            const int32_t workdayCount = rangeUtc.getWorkdayCount(utc);
            const float partialWorkdayCount = rangeUtc.getPartialWorkdayCount(utc);

            THEN("its workday count is expected to be 5") {
                CHECK(workdayCount == 5);
            }

            THEN("its partial workday count is expected to be 5") {
                CHECK(partialWorkdayCount == 5.0f);
            }
        }

        WHEN("getting partitioned range that spans 4 days") {
            const auto rangePartition = rangeUtc.getRangePartitionByDays(1, 4, utc);
            const int32_t workdayCount = rangePartition.getWorkdayCount(utc);
            const float partialWorkdayCount = rangePartition.getPartialWorkdayCount(utc);

            THEN("its workday count is expected to be 4") {
                CHECK(workdayCount == 4);
            }

            THEN("its partial workday count is expected to be 4") {
                CHECK(partialWorkdayCount == 4.0f);
            }
        }
    }

    GIVEN("a week range") {
        const auto startUtc = Time::StringToInstant("2025-02-03T00:00:00Z");
        const auto endUtc = Time::StringToInstant("2025-02-10T00:00:00Z");

        const Time::Range rangeUtc(startUtc, endUtc);

        const std::chrono::tzdb& tzdb = std::chrono::get_tzdb();
        const std::chrono::time_zone* utc = tzdb.locate_zone("UTC");

        WHEN("getting workday day count") {
            const int32_t workdayCount = rangeUtc.getWorkdayCount(utc);
            const float partialWorkdayCount = rangeUtc.getPartialWorkdayCount(utc);

            THEN("its workday count is expected to be 5") {
                CHECK(workdayCount == 5);
            }

            THEN("its partial workday count is expected to be 5") {
                CHECK(partialWorkdayCount == 5.0f);
            }
        }

        WHEN("getting partitioned range that spans 5 days") {
            const auto rangePartition = rangeUtc.getRangePartitionByDays(1, 5, utc);
            const int32_t workdayCount = rangePartition.getWorkdayCount(utc);
            const float partialWorkdayCount = rangePartition.getPartialWorkdayCount(utc);

            THEN("its workday count is expected to be 4") {
                CHECK(workdayCount == 4);
            }

            THEN("its partial workday count is expected to be 4") {
                CHECK(partialWorkdayCount == 4.0f);
            }
        }
    }

    GIVEN("a two week range") {
        const auto startUtc = Time::StringToInstant("2025-02-03T00:00:00Z");
        const auto endUtc = Time::StringToInstant("2025-02-17T00:00:00Z");

        const Time::Range rangeUtc(startUtc, endUtc);

        const std::chrono::tzdb& tzdb = std::chrono::get_tzdb();
        const std::chrono::time_zone* utc = tzdb.locate_zone("UTC");

        WHEN("getting workday day count") {
            const int32_t workdayCount = rangeUtc.getWorkdayCount(utc);
            const float partialWorkdayCount = rangeUtc.getPartialWorkdayCount(utc);

            THEN("its workday count is expected to be 10") {
                CHECK(workdayCount == 10);
            }

            THEN("its partial workday count is expected to be 10") {
                CHECK(partialWorkdayCount == 10.0f);
            }
        }
    }

    GIVEN("a two and a half workday range (starting at 12:00)") {
        const auto startUtc = Time::StringToInstant("2025-02-03T12:00:00Z");
        const auto endUtc = Time::StringToInstant("2025-02-06T00:00:00Z");

        const Time::Range rangeUtc(startUtc, endUtc);

        const std::chrono::tzdb& tzdb = std::chrono::get_tzdb();
        const std::chrono::time_zone* utc = tzdb.locate_zone("UTC");

        WHEN("getting workday day count") {
            const int32_t workdayCount = rangeUtc.getWorkdayCount(utc);
            const float partialWorkdayCount = rangeUtc.getPartialWorkdayCount(utc);

            THEN("its workday count is expected to be 3") {
                CHECK(workdayCount == 3);
            }

            THEN("its partial workday count is expected to be 2.5") {
                CHECK(partialWorkdayCount == 2.5f);
            }
        }
    }

    GIVEN("a two and a half day range (starting at 12:00) that includes one weekend") {
        const auto startUtc = Time::StringToInstant("2025-02-02T12:00:00Z");
        const auto endUtc = Time::StringToInstant("2025-02-05T00:00:00Z");

        const Time::Range rangeUtc(startUtc, endUtc);

        const std::chrono::tzdb& tzdb = std::chrono::get_tzdb();
        const std::chrono::time_zone* utc = tzdb.locate_zone("UTC");

        WHEN("getting workday day count") {
            const int32_t workdayCount = rangeUtc.getWorkdayCount(utc);
            const float partialWorkdayCount = rangeUtc.getPartialWorkdayCount(utc);

            THEN("its workday count is expected to be 2") {
                CHECK(workdayCount == 2);
            }

            THEN("its partial workday count is expected to be 2") {
                CHECK(partialWorkdayCount == 2.0f);
            }
        }
    }

    GIVEN("a three workday range that spans four days (starting and ending at 12:00)") {
        const auto startUtc = Time::StringToInstant("2025-02-03T12:00:00Z");
        const auto endUtc = Time::StringToInstant("2025-02-06T12:00:00Z");

        const Time::Range rangeUtc(startUtc, endUtc);

        const std::chrono::tzdb& tzdb = std::chrono::get_tzdb();
        const std::chrono::time_zone* utc = tzdb.locate_zone("UTC");

        WHEN("getting workday day count") {
            const int32_t workdayCount = rangeUtc.getWorkdayCount(utc);
            const float partialWorkdayCount = rangeUtc.getPartialWorkdayCount(utc);

            THEN("its workday count is expected to be 4") {
                CHECK(workdayCount == 4);
            }

            THEN("its partial workday count is expected to be 3") {
                CHECK(partialWorkdayCount == 3.0f);
            }
        }
    }

    GIVEN("a three day range that spans four days (starting and ending at 12:00) and includes one weekend") {
        const auto startUtc = Time::StringToInstant("2025-02-02T12:00:00Z");
        const auto endUtc = Time::StringToInstant("2025-02-05T12:00:00Z");

        const Time::Range rangeUtc(startUtc, endUtc);

        const std::chrono::tzdb& tzdb = std::chrono::get_tzdb();
        const std::chrono::time_zone* utc = tzdb.locate_zone("UTC");

        WHEN("getting workday day count") {
            const int32_t workdayCount = rangeUtc.getWorkdayCount(utc);
            const float partialWorkdayCount = rangeUtc.getPartialWorkdayCount(utc);

            THEN("its workday count is expected to be 3") {
                CHECK(workdayCount == 3);
            }

            THEN("its partial workday count is expected to be 2.5") {
                CHECK(partialWorkdayCount == 2.5f);
            }
        }
    }

    GIVEN("a three day range that spans four days (starting and ending at 12:00) and includes one weekend") {
        const auto startUtc = Time::StringToInstant("2025-02-02T12:00:00Z");
        const auto endUtc = Time::StringToInstant("2025-02-05T12:00:00Z");

        const Time::Range rangeUtc(startUtc, endUtc);

        const std::chrono::tzdb& tzdb = std::chrono::get_tzdb();
        const std::chrono::time_zone* utc = tzdb.locate_zone("UTC");

        WHEN("getting workday day count") {
            const int32_t workdayCount = rangeUtc.getWorkdayCount(utc);
            const float partialWorkdayCount = rangeUtc.getPartialWorkdayCount(utc);

            THEN("its workday count is expected to be 3") {
                CHECK(workdayCount == 3);
            }

            THEN("its partial workday count is expected to be 2.5") {
                CHECK(partialWorkdayCount == 2.5f);
            }
        }
    }

    GIVEN("a six day range that spans seven days (starting and ending at 12:00) at zone Europe/Riga (includes DST +02:00 -> +03:00)") {
        const auto startUtc = Time::StringToInstant("2025-03-27T12:00:00+02:00");
        const auto endUtc = Time::StringToInstant("2025-04-02T12:00:00+03:00");

        const Time::Range rangeUtc(startUtc, endUtc);

        const std::chrono::tzdb& tzdb = std::chrono::get_tzdb();
        const std::chrono::time_zone* zone = tzdb.locate_zone("Europe/Riga");

        WHEN("getting day count") {
            const int32_t workdayCount = rangeUtc.getDayCount(zone, 0x7f);
            const float partialWorkdayCount = rangeUtc.getPartialDayCount(zone, 0x7f);

            THEN("its workday count is expected to be 7") {
                CHECK(workdayCount == 7);
            }

            THEN("its partial workday count is expected to be 6") {
                CHECK(partialWorkdayCount == 6.0f);
            }
        }
    }
}
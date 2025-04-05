#include "doctest.h"

#include <chrono>

#include "Time/Range.h"
#include "Time/RangeCollection.h"

SCENARIO("time range intersections") {
    GIVEN("two intersecting and non-adjacent ranges") {
        const auto start1 = Time::StringToInstant("2025-02-01T00:00:00Z");
        const auto end1 = Time::StringToInstant("2025-03-01T00:00:00Z");
        const auto start2 = Time::StringToInstant("2025-02-28T23:59:59Z");
        const auto end2 = Time::StringToInstant("2025-04-01T00:00:00Z");

        const Time::Range range1(start1, end1);
        const Time::Range range2(start2, end2);

        WHEN("checking if they are adjacent") {
            const bool adjacent1 = range1.isAdjacentTo(range2);
            const bool adjacent2 = range2.isAdjacentTo(range1);

            THEN("it must be false") {
                CHECK(!adjacent1);
                CHECK(!adjacent2);
            }
        }

        WHEN("checking their intersection") {
            const bool intersects1 = range1.intersects(range2);
            const bool intersects2 = range2.intersects(range1);

            THEN("it must be true") {
                CHECK(intersects1);
                CHECK(intersects2);
            }
        }

        WHEN("getting their intersection") {
            const auto intersection1 = range1.getIntersection(range2);
            const auto intersection2 = range2.getIntersection(range1);

            THEN("there must be an intersection") {
                CHECK(intersection1.get() != nullptr);
                CHECK(intersection2.get() != nullptr);
            }

            THEN("intersections must be equal") {
                CHECK(*intersection1 == *intersection2);
            }

            THEN("intersection should have correct start and end") {
                CHECK(intersection1->start() == start2);
                CHECK(intersection1->end() == end1);
            }
        }
    }

    GIVEN("two non-intersecting and non-adjacent ranges") {
        const auto start1 = Time::StringToInstant("2025-02-01T00:00:00Z");
        const auto end1 = Time::StringToInstant("2025-03-01T00:00:00Z");
        const auto start2 = Time::StringToInstant("2025-03-01T00:00:01Z");
        const auto end2 = Time::StringToInstant("2025-04-01T00:00:00Z");

        const Time::Range range1(start1, end1);
        const Time::Range range2(start2, end2);

        WHEN("checking if they are adjacent") {
            const bool adjacent1 = range1.isAdjacentTo(range2);
            const bool adjacent2 = range2.isAdjacentTo(range1);

            THEN("it must be false") {
                CHECK(!adjacent1);
                CHECK(!adjacent2);
            }
        }

        WHEN("checking their intersection") {
            const bool intersects1 = range1.intersects(range2);
            const bool intersects2 = range2.intersects(range1);

            THEN("it must be false") {
                CHECK(!intersects1);
                CHECK(!intersects2);
            }
        }

        WHEN("getting their intersection") {
            const auto intersection1 = range1.getIntersection(range2);
            const auto intersection2 = range2.getIntersection(range1);

            THEN("there must not be an intersection") {
                CHECK(!intersection1.get());
                CHECK(!intersection2.get());
            }

            THEN("intersections must be equal") {
                CHECK(intersection1.get() == intersection2.get());
            }
        }
    }

    GIVEN("two non-intersecting and adjacent ranges") {
        const auto start1 = Time::StringToInstant("2025-02-01T00:00:00Z");
        const auto end1 = Time::StringToInstant("2025-03-01T00:00:00Z");
        const auto start2 = Time::StringToInstant("2025-03-01T00:00:00Z");
        const auto end2 = Time::StringToInstant("2025-04-01T00:00:00Z");

        const Time::Range range1(start1, end1);
        const Time::Range range2(start2, end2);

        WHEN("checking if they are adjacent") {
            const bool adjacent1 = range1.isAdjacentTo(range2);
            const bool adjacent2 = range2.isAdjacentTo(range1);

            THEN("it must be true") {
                CHECK(adjacent1);
                CHECK(adjacent2);
            }
        }

        WHEN("checking their intersection") {
            const bool intersects1 = range1.intersects(range2);
            const bool intersects2 = range2.intersects(range1);

            THEN("it must be false") {
                CHECK(!intersects1);
                CHECK(!intersects2);
            }
        }

        WHEN("getting their intersection") {
            const auto intersection1 = range1.getIntersection(range2);
            const auto intersection2 = range2.getIntersection(range1);

            THEN("there must not be an intersection") {
                CHECK(!intersection1.get());
                CHECK(!intersection2.get());
            }

            THEN("intersections must be equal") {
                CHECK(intersection1.get() == intersection2.get());
            }
        }
    }

    GIVEN("a range and a range collection that intersects") {
        const auto start1 = Time::StringToInstant("2025-02-01T00:00:00Z");
        const auto end1 = Time::StringToInstant("2025-03-01T00:00:00Z");
        const auto start2 = Time::StringToInstant("2025-03-03T00:00:00Z");
        const auto end2 = Time::StringToInstant("2025-04-01T00:00:00Z");

        const auto start3 = Time::StringToInstant("2025-03-02T00:00:00Z");
        const auto end3 = Time::StringToInstant("2025-03-04T00:00:00Z");

        const Time::Range range1(start1, end1);
        const Time::Range range2(start2, end2);

        const Time::Range range(start3, end3);

        Time::RangeCollection collection(2);
        collection.add(range1);
        collection.add(range2);

        WHEN("checking their intersection") {
            const bool intersects1 = collection.intersects(range);
            const bool intersects2 = range.intersects(collection);

            THEN("it must be true") {
                CHECK(intersects1);
                CHECK(intersects2);
            }
        }
    }

    GIVEN("a range and a range collection that does not intersect") {
        const auto start1 = Time::StringToInstant("2025-02-01T00:00:00Z");
        const auto end1 = Time::StringToInstant("2025-03-01T00:00:00Z");
        const auto start2 = Time::StringToInstant("2025-03-03T00:00:00Z");
        const auto end2 = Time::StringToInstant("2025-04-01T00:00:00Z");

        const auto start3 = Time::StringToInstant("2025-03-02T00:00:00Z");
        const auto end3 = Time::StringToInstant("2025-03-03T00:00:00Z");

        const Time::Range range1(start1, end1);
        const Time::Range range2(start2, end2);

        const Time::Range range(start3, end3);

        Time::RangeCollection collection(2);
        collection.add(range1);
        collection.add(range2);

        WHEN("checking their intersection") {
            const bool intersects1 = collection.intersects(range);
            const bool intersects2 = range.intersects(collection);

            THEN("it must be false") {
                CHECK(!intersects1);
                CHECK(!intersects2);
            }
        }
    }
}
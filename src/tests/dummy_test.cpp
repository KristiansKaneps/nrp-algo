#include "doctest.h"

#ifdef _WIN32
#include <windows.h>
int sleep(const unsigned int seconds) {
    Sleep(seconds * 1000);
    return 0;
}
#else
#include <unistd.h>
#endif

SCENARIO("sleep a second") {
    GIVEN("a one second sleep duration") {
        unsigned int duration = 1;

        WHEN("call sleep with this duration") {
            int ret = sleep(duration);

            THEN("it's expected nobody interrupted our rest") {
                CHECK(ret == 0);
            }
        }
    }
}
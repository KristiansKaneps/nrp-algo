#ifndef APPLICATIONSTATE_H
#define APPLICATIONSTATE_H

#include <cstdint>
#include <limits>

#include "ConcurrentData.h"

#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Employee.h"
#include "Domain/Entities/Day.h"
#include "Domain/Entities/Skill.h"

#include "Constraints/Constraint.h"

struct RenderCache {
    uint64_t maxTotalWorkDuration = 0;
    uint64_t minTotalWorkDuration = std::numeric_limits<uint64_t>::max();
    uint64_t *employeeTotalWorkDuration{};

    bool *dayCoverageValid{};

    BitArray::BitArray *xw;

    std::vector<bool> weekends;

    struct DayAvailability {
        struct Region {
            float start, width;
        };

        Availability::Type type;
        Region region;
    };

    std::vector<DayAvailability>* employeeAvailabilityPerDay;

    ~RenderCache() {
        delete[] employeeTotalWorkDuration;
        delete[] dayCoverageValid;
        delete xw;
        delete[] employeeAvailabilityPerDay;
    }
};

struct AppState {
    Score::Score score;
    State::State<Shift, Employee, Day, Skill> state;
    std::vector<Constraints::Constraint<Shift, Employee, Day, Skill> *> constraints;

    RenderCache renderCache;

    Statistics::ScoreStatistics scoreStatistics {};

    bool localSearchDone = false;
};

inline AppState *gp_AppState = nullptr;

#endif //APPLICATIONSTATE_H

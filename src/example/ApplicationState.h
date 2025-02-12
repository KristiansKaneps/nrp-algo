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

struct AppState {
    Score::Score score;
    State::State<Shift, Employee, Day, Skill> state;
    std::vector<Constraints::Constraint<Shift, Employee, Day, Skill> *> constraints;

    struct RenderCache {
        uint64_t maxTotalWorkDuration = 0;
        uint64_t minTotalWorkDuration = std::numeric_limits<uint64_t>::max();
        uint64_t *employeeTotalWorkDuration{};

        bool *dayCoverageValid{};

        BitArray::BitArray *xw;

        ~RenderCache() {
            delete[] employeeTotalWorkDuration;
            delete[] dayCoverageValid;
            delete xw;
        }
    };

    RenderCache renderCache;
};

inline AppState *gp_AppState = nullptr;

#endif //APPLICATIONSTATE_H

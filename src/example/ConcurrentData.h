#ifndef CONCURRENTDATA_H
#define CONCURRENTDATA_H

#include <mutex>

#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Employee.h"
#include "Domain/Entities/Day.h"
#include "Domain/Entities/Skill.h"

#include "State/State.h"
#include "Score/Score.h"

#include "Statistics/ScoreStatistics.h"

using namespace Domain;

enum class LocalSearchUpdateFlag : uint8_t {
    NONE = 0,
    PENDING = 1,
};

struct LocalSearchUpdate {
    State::State<Shift, Employee, Day, Skill> state;
    Score::Score score;
    Statistics::ScoreStatistics scoreStatistics;
    bool localSearchDone = false;
};

inline volatile bool g_LocalSearchShouldStop = false; // single bool should be atomic on most architectures
inline volatile auto g_UpdateFlag = LocalSearchUpdateFlag::NONE; // single byte should be atomic on most architectures
inline LocalSearchUpdate *gp_Update = nullptr;

inline std::mutex g_ConcurrentDataMutex{};

#endif //CONCURRENTDATA_H

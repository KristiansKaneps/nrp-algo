#define LOCALSEARCH_DEBUG


#include "Application.h"
#include "Database.h"
#include "UserDataAccumulator.h"
#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Employee.h"
#include "Domain/Entities/Day.h"
#include "Domain/Entities/Skill.h"

#include "Domain/Constraints/NoOverlapConstraint.h"
#include "Domain/Constraints/RequiredSkillConstraint.h"

#include "Search/LocalSearch.h"
#include "State/State.h"

#include "Time/RangeCollection.h"

#include "ConcurrentData.h"

#include "WindowsUtils.c"

#include <thread>

using namespace Domain;

static void onNewBest(const Search::LocalSearch<Shift, Employee, Day, Skill> &localSearch) {
    gp_Update = new(gp_Update) LocalSearchUpdate {
        .state = localSearch.getBestState(),
        .score = localSearch.getBestScore(),
    };
    g_UpdateFlag = LocalSearchUpdateFlag::NEW_BEST_AVAILABLE;
}

void solve() {
    Search::LocalSearch localSearch(&gp_AppState->state, gp_AppState->constraints);

    bool mutexWasLocked = false;

    const auto initialScore = localSearch.evaluateCurrentBestState();
    std::cout << "Initial score: " << initialScore << std::endl;
    while (!localSearch.isDone() && !g_LocalSearchShouldStop) {
        if (localSearch.step() || mutexWasLocked) {
            if (g_ConcurrentDataMutex.try_lock()) {
                onNewBest(localSearch);
                g_ConcurrentDataMutex.unlock();
                mutexWasLocked = false;
            } else {
                mutexWasLocked = true;
            }
        }
    }

    if (mutexWasLocked) {
        g_ConcurrentDataMutex.lock();
        onNewBest(localSearch);
        g_ConcurrentDataMutex.unlock();
    }

    const auto bestScore = localSearch.evaluateCurrentBestState();
    std::cout << "Best score: " << bestScore << "  Delta: " << (bestScore - initialScore) << std::endl;
}

int main(int argc, char **argv) {
    SetConsoleOutputToUTF8();

    {
        std::tm tm = {};
        std::stringstream ss("2024-12-01T00:00:00Z");
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
        auto start = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        ss = std::stringstream("2025-01-01T00:00:00Z");
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
        auto end = std::chrono::system_clock::from_time_t(std::mktime(&tm));

        const Time::Range range(start, end);

        const uint32_t shiftCount = 4;
        const uint32_t employeeCount = 50;
        const uint32_t dayCount = 31;
        const uint32_t skillCount = 4;

        std::allocator<Day> dayAllocator;
        std::allocator<Employee> employeeAllocator;
        std::allocator<Shift> shiftAllocator;
        std::allocator<Skill> skillAllocator;

        auto *shifts = shiftAllocator.allocate(shiftCount);
        auto *employees = employeeAllocator.allocate(employeeCount);
        auto *days = dayAllocator.allocate(dayCount);
        auto *skills = skillAllocator.allocate(skillCount);

        const Time::DailyInterval interval1(480, 480);
        const Time::DailyInterval interval2(960, 480);
        const Time::DailyInterval interval3(480, 1440);
        const Time::DailyInterval interval4(1200, 1440);

        new(shifts + 0) Shift(0, interval1, "E");
        new(shifts + 1) Shift(1, interval2, "L");
        new(shifts + 2) Shift(2, interval3, "DN");
        new(shifts + 3) Shift(3, interval4, "ND");

        for (uint32_t i = 0; i < employeeCount; ++i) { new(employees + i) Employee(i); }

        for (uint32_t i = 0; i < dayCount; ++i) {
            new(days + i) Day(i, range.getDayRangeAt(i, std::chrono::current_zone()));
            // std::cout << days[i].range().start() << ' ' << days[i].range().end() << std::endl;
        }

        for (uint32_t i = 0; i < skillCount; ++i) { new(skills + i) Skill(i, std::to_string(i)); }

        employees[2].addSkill(0, 1.0f);
        employees[2].addSkill(3, 1.0f);
        employees[2].addSkill(1, 1.0f);

        shifts[0].addRequiredAllSkill(0, 1.0f);
        shifts[0].addRequiredOneSkill(1, 1.0f);
        shifts[0].addRequiredOneSkill(2, 1.0f);

        shifts[1].addRequiredAllSkill(1, 1.0f);

        shifts[2].addRequiredAllSkill(2, 1.0f);

        shifts[3].addRequiredAllSkill(3, 1.0f);
        shifts[3].addRequiredAllSkill(0, 0.5f);
        shifts[3].addRequiredAllSkill(1, 1.0f);
        shifts[3].addRequiredOneSkill(0, 1.0f);
        shifts[3].addRequiredOneSkill(1, 1.0f);

        auto *axisX = new Axes::Axis(shifts, shiftCount);
        auto *axisY = new Axes::Axis(employees, employeeCount);
        auto *axisZ = new Axes::Axis(days, dayCount);
        auto *axisW = new Axes::Axis(skills, skillCount);

        State::State<Shift, Employee, Day, Skill> state(
            range,
            axisX,
            axisY,
            axisZ,
            axisW
        );

        for (int i = 0; i < shiftCount; i++) { state.set(i, 0, 0, 0); }
        // for (int i = 0; i < dayCount; i++) { state.set(3, 2, i, 1); }
        // for (int i = 0; i < dayCount; i++) { state.set(2, 2, i, 1); }

        // state.random(0.2);

        state.printSize();

        auto *noOverlapConstraint = new Constraints::NoOverlapConstraint(state.x());
        auto *requiredSkillConstraint = new Constraints::RequiredSkillConstraint(state.x(), state.y(), state.w());

        const auto constraints = std::vector<Constraints::Constraint<Shift, Employee, Day, Skill> *> {
            noOverlapConstraint,
            requiredSkillConstraint,
        };

        std::cout << "State 1 score: " << Evaluation::evaluateState(state, constraints) << std::endl;
        state.clearAll();
        state.random(0.2);

        gp_AppState = new AppState{
            .score = Evaluation::evaluateState(state, constraints),
            .state = state,
            .constraints = constraints,
        };

        gp_Update = new LocalSearchUpdate{
            .state = gp_AppState->state,
            .score = gp_AppState->score,
        };
    }

    std::thread solverThread(solve);

    // const auto now = std::chrono::system_clock::now();
    // const std::chrono::duration<int64_t, std::nano> second(1000000000);
    //
    // Time::Range range1(now, now + second);
    // Time::Range range2(now + 2 * second, now + 4 * second);
    //
    // Time::RangeCollection collection(2);
    // collection.add(range1);
    // collection.add(range2);
    //
    // Time::Ray ray = now + second;
    // std::cout << "Ray intersects range 1 & 2: " << (ray.intersects(range1) ? "true" : "false") << ' ' << (
    //     ray.intersects(range2) ? "true" : "false") << std::endl;
    // std::cout << "Ray intersects range collection: " << (ray.intersects(collection) ? "true" : "false") << std::endl;
    //
    // // std::cout << "Press any key to exit..." << std::endl;
    // // std::cin.get();
    // // std::cout << "Exited." << std::endl;

    // Database database;
    // if (database.isConnected()) {
    //     // database.execute("select u.name, u.surname from users u");
    //     // for (uint32_t i = 0; i < database.tupleCount(); ++i) {
    //     //     std::cout << database.value(i, 0) << " " << database.value(i, 1) << std::endl;
    //     // }
    //
    //     const std::string rangeStart = Time::InstantToString(range.start());
    //     const std::string rangeEnd = Time::InstantToString(range.end());
    //
    //     std::cout << "Range start as string: " << rangeStart << std::endl;
    //     const auto parsedInstant = Time::StringToInstant(rangeStart);
    //     std::cout << "Range start parsed: " << parsedInstant << std::endl;
    //
    //     database.execute(
    //         "select u.id, u.name, u.surname, u.email, a.type, a.unavailability_subtype, a.start_date, a.end_date "
    //         "from users u join organizational_unit_users ouu on ouu.user_id = u.id and ouu.organizational_unit_id = 'b2ecb1e5-0cf6-41e9-95cf-07a30dd14ebd' "
    //         "left join availabilities a on a.user_id = u.id and a.start_date < '" + rangeEnd + "' and a.end_date > '" +
    //         rangeStart + '\''
    //     );
    //
    //     std::cout << "Tuple count: " << database.tupleCount() << std::endl;
    //     std::cout << "Attribute count: " << database.attributeCount() << std::endl;
    //     std::cout << "Error: " << database.errorMessage() << std::endl;
    //
    //     UserData::Accumulator userAccumulator;
    //
    //     for (uint32_t i = 0; i < database.tupleCount(); ++i) {
    //         const char *userId = database.value(i, 0);
    //         const char *userName = database.value(i, 1);
    //         const char *userSurname = database.value(i, 2);
    //         const char *userEmail = database.value(i, 3);
    //         const auto availabilityType = static_cast<UserData::AvailabilityType>(atoi(database.value(i, 4)));
    //         const auto unavailabilitySubtype = static_cast<UserData::UnavailabilitySubtype>(atoi(database.value(i, 5)));
    //
    //         userAccumulator.addUserFullName(userEmail, userName, userSurname);
    //
    //         if (!database.isNull(i, 6) && !database.isNull(i, 7)) {
    //             const char *availabilityRangeStartStr = database.value(i, 6);
    //             const char *availabilityRangeEndStr = database.value(i, 7);
    //             std::cout << "Raw: " << availabilityRangeStartStr << ", " << availabilityRangeEndStr << std::endl;
    //             const auto availabilityRangeStart = Time::StringToInstant(availabilityRangeStartStr);
    //             const auto availabilityRangeEnd = Time::StringToInstant(availabilityRangeEndStr);
    //             std::cout << "Parsed: " << availabilityRangeStart << ", " << availabilityRangeEnd << std::endl;
    //             const Time::Range availabilityRange(availabilityRangeStart, availabilityRangeEnd);
    //             userAccumulator.addAvailability(userEmail, availabilityRange, availabilityType, unavailabilitySubtype);
    //         }
    //     }
    // }

    Application app(1280, 720, "Example Application");
    app.start();

    solverThread.join();
    delete gp_Update;
    delete gp_AppState;
    return 0;
}

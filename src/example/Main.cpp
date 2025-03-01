#define LOCALSEARCH_DEBUG


#include "Application.h"

#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Employee.h"
#include "Domain/Entities/Day.h"
#include "Domain/Entities/Skill.h"

#include "Search/LocalSearch.h"

#include "Time/RangeCollection.h"

#include "ConcurrentData.h"

#include "WindowsUtils.c"

#include <thread>

#include "Example.h"

using namespace Domain;

static void onNewBest(const Search::LocalSearch<Shift, Employee, Day, Skill> &localSearch) {
    gp_Update = new(gp_Update) LocalSearchUpdate {
        .state = localSearch.getBestState(),
        .score = localSearch.getBestScore(),
    };
    g_UpdateFlag = LocalSearchUpdateFlag::NEW_BEST_AVAILABLE;
}

void solve() {
    Search::LocalSearch localSearch(&gp_AppState->state, gp_AppState->constraints, gp_AppState->heuristicProvider);

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

// ReSharper disable once CppDFAConstantFunctionResult
int main(int argc, char **argv) {
    SetConsoleOutputToUTF8();

    Example::create();

    std::thread solverThread(solve);

    // const auto now = std::chrono::system_clock::now();
    // constexpr std::chrono::duration<int64_t, std::nano> second(1000000000);

    // std::cout << "Range: " << range << std::endl;
    // const auto d1 = range.getDayAt(1, timeZone);
    // std::cout << "Day 1: " << d1 << std::endl;
    // const auto s1 = gp_AppState->state.x()[0];
    // const auto s2 = gp_AppState->state.x()[1];
    // const auto s3 = gp_AppState->state.x()[2];
    // const auto s4 = gp_AppState->state.x()[3];
    // const auto r1 = s1.interval().toRange(d1);
    // const auto r2 = s2.interval().toRange(d1);
    // const auto r3 = s3.interval().toRange(d1);
    // const auto r4 = s4.interval().toRange(d1);
    // std::cout << "Interval 1: " << s1.interval() << ", range 1: " << r1 << std::endl;
    // std::cout << "Interval 2: " << s2.interval() << ", range 2: " << r2 << std::endl;
    // std::cout << "Interval 3: " << s3.interval() << ", range 3: " << r3 << std::endl;
    // std::cout << "Interval 4: " << s4.interval() << ", range 4: " << r4 << std::endl;

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

    Application app(1280, 720, "Example Application");
    app.start();

    solverThread.join();
    delete gp_Update;
    delete gp_AppState;
    return 0;
}

#define ENABLE_SCORE_STATISTICS
#define LOCALSEARCH_DEBUG
// #define MEMORY_USAGE_DEBUG


#include "Application.h"

#include "GUI/Scenes/TimetableScene.h"

#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Employee.h"
#include "Domain/Entities/Day.h"
#include "Domain/Entities/Skill.h"

#include "Search/LocalSearch.h"

#include "ConcurrentData.h"

#include "WindowsUtils.c"

#include <thread>

#include "Example.h"

#ifdef MEMORY_USAGE_DEBUG
#include "Memory/AllocationMetrics.h"
#endif

using namespace Domain;

static void onNewBest(const Search::LocalSearch<Shift, Employee, Day, Skill> &localSearch) {
    gp_Update = new(gp_Update) LocalSearchUpdate {
        .state = localSearch.getBestState(),
        .score = localSearch.getBestScore(),
    };
    g_UpdateFlag = LocalSearchUpdateFlag::NEW_BEST_AVAILABLE;

    #ifdef MEMORY_USAGE_DEBUG
    Memory::printMemoryUsage();
    #endif
}

void solve() {
    using std::chrono::high_resolution_clock;
    using std::chrono_literals::operator ""s;

    Search::LocalSearch localSearch(&gp_AppState->state, gp_AppState->constraints, gp_AppState->heuristicProvider);

    bool mutexWasLocked = false;

    const auto initialScore = localSearch.evaluateCurrentBestState();
    std::cout << "Initial score: " << initialScore << std::endl;

    const auto start = high_resolution_clock::now();

    localSearch.startStatistics();
    while (!localSearch.isDone() && !g_LocalSearchShouldStop) {
        if ((!IsKeyDown(KEY_ESCAPE) && localSearch.step()) || mutexWasLocked) {
            if (g_ConcurrentDataMutex.try_lock()) {
                onNewBest(localSearch);
                g_ConcurrentDataMutex.unlock();
                mutexWasLocked = false;
            } else {
                mutexWasLocked = true;
            }
        }
    }
    localSearch.endStatistics();

    const auto end = high_resolution_clock::now();
    const auto diff = (end - start) / 1s;

    std::cout << "Best solution found in " << (diff / 60) << "min " << (diff % 60) << "s" << std::endl;

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

    Application app(1280, 720, "NRP Algo");
    app.setScene<GUI::TimetableScene>();
    // Application app(475, 163, "NRP Algo");
    app.start();

    solverThread.join();
    delete gp_Update;
    delete gp_AppState;
    return 0;
}

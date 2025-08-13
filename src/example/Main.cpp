#define ENABLE_SCORE_STATISTICS
#define ENABLE_SCORE_STATISTICS_SCENE
// #define LOCALSEARCH_DEBUG
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

#include <cassert>
#include <thread>
#include <unordered_set>

#include "Example.h"

#include "IO/StateFile.h"
#include "NrpProblemInstances/NrpProblemSerializer.h"

#ifdef MEMORY_USAGE_DEBUG
#include "Memory/AllocationMetrics.h"
#endif

using namespace Domain;

static bool gs_Cli = true; // Is this app running as a CLI?

static void updateConcurrentData(const Search::LocalSearch<Shift, Employee, Day, Skill> &localSearch) {
    gp_Update->state = localSearch.getBestState();
    gp_Update->score = localSearch.getBestScore();
    gp_Update->localSearchDone = localSearch.isDone();
    #ifdef ENABLE_SCORE_STATISTICS_SCENE
    gp_Update->scoreStatistics = localSearch.scoreStatistics();
    #endif
    g_UpdateFlag = LocalSearchUpdateFlag::PENDING;

    #ifdef MEMORY_USAGE_DEBUG
    Memory::printMemoryUsage();
    #endif
}

void solve() {
    using std::chrono::high_resolution_clock;
    using std::chrono_literals::operator ""s;

    // gp_AppState->state.random(0.1f);
    Search::LocalSearch localSearch(&gp_AppState->state, gp_AppState->constraints);

    bool mutexWasLocked = false;

    const auto initialScore = localSearch.evaluateCurrentBestState();
    std::cout << "Initial score: " << initialScore << std::endl;

    const auto start = high_resolution_clock::now();

    if (gs_Cli) [[likely]] {
        localSearch.startStatistics();
        while (!localSearch.isDone() && !g_LocalSearchShouldStop) {
            localSearch.step();
        }
        localSearch.endStatistics();
    } else {
        localSearch.startStatistics();
        while (!localSearch.isDone() && !g_LocalSearchShouldStop) {
            if ((!IsKeyDown(KEY_ESCAPE) && localSearch.step()) || mutexWasLocked) {
                if (g_ConcurrentDataMutex.try_lock()) {
                    updateConcurrentData(localSearch);
                    g_ConcurrentDataMutex.unlock();
                    mutexWasLocked = false;
                } else {
                    mutexWasLocked = true;
                }
            }
        }
        localSearch.endStatistics();
    }

    const auto end = high_resolution_clock::now();
    const auto diff = (end - start) / 1s;

    std::cout << "Best solution found in " << (diff / 60) << "min " << (diff % 60) << "s" << std::endl;

    if (!gs_Cli) [[unlikely]] {
        g_ConcurrentDataMutex.lock();
        updateConcurrentData(localSearch);
        g_ConcurrentDataMutex.unlock();
    }

    const auto bestScore = localSearch.evaluateCurrentBestState();
    std::cout << "Best score: " << bestScore << "  Delta: " << (bestScore - initialScore) << std::endl;

    {
        IO::StatisticsFile scoreStatisticsFile("score_statistics.csv");
        localSearch.scoreStatistics().write(scoreStatisticsFile);
        IO::StateFile stateFile("solution.txt");
        auto serializer = NrpProblemInstances::NrpProblemSerializer();
        serializer.serialize(stateFile, localSearch.getBestState());
    }
}

// ReSharper disable once CppDFAConstantFunctionResult
int main(int argc, char **argv) {
    SetConsoleOutputToUTF8();

    std::unordered_set<std::string_view> args;
    args.reserve(argc - 1);

    // First argument is path to executable.
    if (argc > 1) {
        std::cout << "Arguments:";
        for (size_t i = 1; i < argc; ++i) {
            args.insert(std::string_view(argv[i]));
            std::cout << " \"" << argv[i] << '"';
        }
        std::cout << std::endl;
    }

    const bool cli = args.contains(std::string_view("--cli"));
    const bool gui = !cli || args.contains(std::string_view("--gui"));
    const bool exitOnFinish = args.contains(std::string_view("--exit-on-finish"));

    assert(((cli && !gui) || (!cli && gui)) && "Simultaneously running as a CLI and GUI is not possible.");

    gs_Cli = cli;

    Example::create();

    std::thread solverThread(solve);

    if (gui) [[unlikely]] {
        Application app(1280, 720, "NRP Algo");
        app.setScene<GUI::TimetableScene>();
        // Application app(475, 163, "NRP Algo");
        app.start(exitOnFinish);
    }

    solverThread.join();
    delete gp_Update;
    delete gp_AppState;
    return 0;
}

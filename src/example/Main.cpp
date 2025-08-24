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
#include <cstdint>
#include <iostream>
#include <thread>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <filesystem>
#include <format>
#include <string_view>
#include <charconv>

#include "Example.h"

#include "IO/StateFile.h"
#include "NrpProblemInstances/NrpProblemSerializer.h"

#ifdef MEMORY_USAGE_DEBUG
#include "Memory/AllocationMetrics.h"
#endif

using namespace Domain;

static bool gs_Cli = true; // Is this app running as a CLI?
static bool gs_Warmup = false; // Is this a warmup app instance?

static void updateConcurrentData(const Search::LocalSearch<Shift, Employee, Day, Skill>& localSearch) {
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

void solve(const std::filesystem::path& outputDirectory, const Search::LocalSearchType localSearchType,
           const uint64_t maxDuration) {
    using std::chrono::high_resolution_clock;
    using std::chrono_literals::operator ""s;

    // gp_AppState->state.random(0.1f);
    Search::LocalSearch localSearch(&gp_AppState->state, gp_AppState->constraints, localSearchType, maxDuration);

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
        bool mutexWasLocked = false;
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

    if (!gs_Warmup) {
        const auto timestampPrefix = String::getTimestampPrefix();
        IO::StatisticsFile scoreStatisticsFile(outputDirectory,
                                               std::format("{}{}_score_statistics.csv", timestampPrefix,
                                                           Search::LocalSearchTypeName(localSearchType)), false);
        localSearch.scoreStatistics().write(scoreStatisticsFile);
        IO::StateFile stateFile(outputDirectory,
                                std::format("{}{}_solution.txt", timestampPrefix,
                                            Search::LocalSearchTypeName(localSearchType)), false);
        auto serializer = NrpProblemInstances::NrpProblemSerializer();
        serializer.serialize(stateFile, localSearch.getBestState());
    }
}

static std::string_view trimBOM(const std::string_view& s) noexcept {
    if (s.size() >= 3 &&
        static_cast<unsigned char>(s[0]) == 0xEF &&
        static_cast<unsigned char>(s[1]) == 0xBB &&
        static_cast<unsigned char>(s[2]) == 0xBF)
        return s.substr(3);
    return s;
}

// ReSharper disable once CppDFAConstantFunctionResult
int main(int argc, char** argv) {
    SetConsoleOutputToUTF8();

    std::unordered_set<std::string_view> args;
    args.reserve(argc - 1);

    // First argument is path to executable.
    if (argc > 1) {
        std::cout << "Arguments:";
        for (size_t i = 1; i < argc; ++i) {
            args.insert(trimBOM(argv[i]));
            std::cout << " \"" << argv[i] << '"';
        }
        std::cout << std::endl;
    }

    bool cli = false;
    bool gui = false;
    bool exitOnFinish = false;
    bool warmup = false;
    std::filesystem::path outputDirectory = std::filesystem::current_path();
    Search::LocalSearchType searchType = Search::LocalSearchType::DLAS;
    uint64_t maxDuration = 0;

    const std::unordered_map<std::string_view, std::function<void()>> argActions = {
        {"--cli", [&] { cli = true; }},
        {"--gui", [&] { gui = true; }},
        {"--exit", [&] { exitOnFinish = true; }},
        {"--exit-on-finish", [&] { exitOnFinish = true; }},
        {"--warmup", [&] { warmup = true; }},
    };

    const std::unordered_map<std::string_view, Search::LocalSearchType> algoMap = {
        {"LAHC", Search::LocalSearchType::LAHC},
        {"DLAS", Search::LocalSearchType::DLAS},
        {"SA", Search::LocalSearchType::SA},
        {"TABU", Search::LocalSearchType::TABU_MOVE},
        {"TABU_MOVE", Search::LocalSearchType::TABU_MOVE},
        {"TABU_STATE", Search::LocalSearchType::TABU_STATE},
    };

    constexpr std::string_view algoPrefix = "--algorithm=";
    constexpr std::string_view outputDirectoryPrefix = "--out=";
    constexpr std::string_view maxDurationPrefix = "--duration="; // In seconds.

    for (const auto& arg: args) {
        if (arg.starts_with(outputDirectoryPrefix)) {
            outputDirectory = std::filesystem::path(arg.substr(outputDirectoryPrefix.size()));
        } else if (arg.starts_with(algoPrefix)) {
            std::array<char, 32> buf{};
            const auto value = arg.substr(algoPrefix.size());
            const auto len = std::min(value.size(), buf.size() - 1);
            for (size_t i = 0; i < len; ++i)
                buf[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(value[i])));

            std::string_view algoName(buf.data(), len);

            if (auto it = algoMap.find(algoName); it != algoMap.end()) {
                searchType = it->second;
            } else {
                std::cerr << "Unknown algorithm: " << algoName << std::endl;
            }
        } else if (arg.starts_with(maxDurationPrefix)) {
            const std::string_view durationAsString = arg.substr(maxDurationPrefix.size());
            auto [ptr, ec] = std::from_chars(durationAsString.data(),
                                             durationAsString.data() + durationAsString.size(),
                                             maxDuration);
            if (ec != std::errc()) {
                std::cerr << "Failed to parse duration: " << durationAsString << std::endl;
            }
        } else {
            if (auto it = argActions.find(arg); it != argActions.end()) {
                it->second();
            }
        }
    }

    assert(((cli && !gui) || (!cli && gui)) && "Simultaneously running as a CLI and GUI is not possible.");

    if (!cli && !gui) cli = true;

    gs_Cli = cli;
    gs_Warmup = warmup;

    Example::create();

    std::thread solverThread(solve, outputDirectory, searchType, maxDuration);

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

#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include "State/State.h"
#include "Constraints/Constraint.h"
#include "Heuristics/HeuristicProvider.h"
#include "Score/Score.h"
#include "Statistics/ScoreStatistics.h"

#include "Search/LocalSearchTask.h"

#include "Search/Implementation/LahcLocalSearchTask.h"
#include "Search/Implementation/DlasLocalSearchTask.h"

namespace Search {
    enum class LocalSearchType { LAHC = 0, DLAS, __COUNT };

    constexpr std::array<std::string_view, static_cast<size_t>(LocalSearchType::__COUNT)> LocalSearchTypeNames = {
        "LAHC", "DLAS"
    };

    constexpr std::string_view LocalSearchTypeName(const LocalSearchType type) {
        return LocalSearchTypeNames[static_cast<size_t>(type)];
    }

    template<typename X, typename Y, typename Z, typename W>
    class LocalSearch {
    public:
        // ReSharper disable CppRedundantQualifier
        explicit LocalSearch(const ::State::State<X, Y, Z, W>* initialState,
                             const std::vector<::Constraints::Constraint<X, Y, Z, W> *>& constraints,
                             const LocalSearchType type = LocalSearchType::DLAS,
                             const uint64_t maxDurationInSeconds = 0) noexcept : m_MaxDurationInSeconds(
                maxDurationInSeconds),
            mp_InitialState(initialState),
            m_Constraints(constraints),
            m_HeuristicProvider(::Heuristics::HeuristicProvider<X, Y, Z, W>(initialState, constraints)) {
            switch (type) {
                case LocalSearchType::LAHC:
                    mp_Task = new Task::LahcLocalSearchTask<X, Y, Z,
                        W>(*initialState, m_Constraints, m_ScoreStatistics);
                    break;
                case LocalSearchType::DLAS:
                    mp_Task = new Task::DlasLocalSearchTask<X, Y, Z,
                        W>(*initialState, m_Constraints, m_ScoreStatistics);
                    break;
                default:
                    mp_Task = new Task::DlasLocalSearchTask<X, Y, Z,
                        W>(*initialState, m_Constraints, m_ScoreStatistics);
                    break;
            }
        }

        // ReSharper restore CppRedundantQualifier

        LocalSearch(const LocalSearch& other) noexcept = default;

        ~LocalSearch() noexcept = default;

        [[nodiscard]] Statistics::ScoreStatistics scoreStatistics() const noexcept { return m_ScoreStatistics; }

        void reset() noexcept {
            m_Done = false;
        }

        void startStatistics() noexcept {
            m_ScoreStatistics.startRecording(mp_Task->getInitialScore());
            m_CountedSteps = 0;
            m_StepsPerSecond = 0;
            m_AverageStepsPerSecond = 0;
            m_StepBatchCount = 0;
            m_StartTime = std::chrono::steady_clock::now();
            m_StepCountTimePoint = m_StartTime;
        }

        void endStatistics() noexcept {
            m_ScoreStatistics.finishRecording();
        }

        /**
         * @return `true` if new best state is found, `false` otherwise
         */
        bool step() noexcept {
            using namespace std::chrono_literals;

            const auto stepStart = std::chrono::steady_clock::now();
            const auto elapsedSeconds = (stepStart - m_StartTime) / 1s;

            // Finalizer
            if (!mp_Task->shouldStep() || !shouldStep(elapsedSeconds)) [[unlikely]] {
                m_Done = true;
                // printBestScore();
                return false;
            }

            mp_Task->step(m_HeuristicProvider);
            m_CountedSteps += 1;

            if (const double delta = std::chrono::duration_cast<std::chrono::duration<double>>(
                stepStart - m_StepCountTimePoint).count(); delta >= 1.0) {
                const double stepsThisInterval = m_CountedSteps / delta;

                // Update running average
                m_StepBatchCount += 1;
                m_AverageStepsPerSecond += (stepsThisInterval - m_AverageStepsPerSecond) / m_StepBatchCount;

                // Reset counters for next interval
                m_CountedSteps = 0;
                m_StepCountTimePoint = stepStart;

                std::cout << "States per second: " << static_cast<int64_t>(stepsThisInterval)
                        << "; average: " << static_cast<int64_t>(m_AverageStepsPerSecond)
                        << "; elapsed time: " << (elapsedSeconds / 60) << "m " << (elapsedSeconds % 60) << 's' <<
                        std::endl;
                std::cout << "Current best score: " << getBestScore()
                        << "; delta: " << getDeltaScore() << std::endl;
            }

            return mp_Task->newBestFound();
        }

        [[nodiscard]] bool shouldStep(const int64_t elapsedSeconds) const noexcept {
            return m_MaxDurationInSeconds != 0 && elapsedSeconds < m_MaxDurationInSeconds;
        }

        [[nodiscard]] bool isDone() const noexcept { return m_Done; }

        // ReSharper disable once CppRedundantQualifier
        ::State::State<X, Y, Z, W> getBestState() const noexcept { return mp_Task->getOutputState(); }
        [[nodiscard]] Score::Score getBestScore() const noexcept { return mp_Task->getOutputScore(); }
        [[nodiscard]] Score::Score getInitialScore() const noexcept { return mp_Task->getInitialScore(); }
        [[nodiscard]] Score::Score getDeltaScore() const noexcept { return getBestScore() - getInitialScore(); }

        [[nodiscard]] const std::chrono::time_point<std::chrono::steady_clock>& getStartTime() const noexcept {
            return m_StartTime;
        }

        [[nodiscard]] double getCurrentStepCountPerSecond() const noexcept {
            return m_StepsPerSecond;
        }

        [[nodiscard]] double getAverageStepsPerSecond() const noexcept {
            return m_AverageStepsPerSecond;
        }

        [[nodiscard]] uint64_t getMaxDurationInSeconds() const noexcept {
            return m_MaxDurationInSeconds;
        }

        [[nodiscard]] Score::Score evaluateCurrentBestState() const noexcept {
            Score::Score score{};
            // ReSharper disable once CppRedundantQualifier
            for (::Constraints::Constraint<X, Y, Z, W>* constraint: m_Constraints)
                score += constraint->evaluate(mp_Task->getOutputState());
            return score;
        }

        void printBestScore() const noexcept {
            std::cout << "Best score: " << mp_Task->getOutputScore() << "; delta=" << (
                mp_Task->getOutputScore() - mp_Task->m_InitScore) << std::endl;
        }

    protected:

    private:
        bool m_Done = false;
        uint64_t m_MaxDurationInSeconds = 0;
        std::chrono::time_point<std::chrono::steady_clock> m_StartTime = std::chrono::time_point_cast<
            std::chrono::nanoseconds>(std::chrono::steady_clock::now());
        uint64_t m_CountedSteps = 0;
        std::chrono::time_point<std::chrono::steady_clock> m_StepCountTimePoint = m_StartTime;
        double m_StepsPerSecond = 0;
        double m_AverageStepsPerSecond = 0;
        int64_t m_StepBatchCount = 0;

        // ReSharper disable once CppRedundantQualifier
        const ::State::State<X, Y, Z, W>* mp_InitialState;
        // ReSharper disable once CppRedundantQualifier
        const std::vector<::Constraints::Constraint<X, Y, Z, W> *> m_Constraints;
        // ReSharper disable once CppRedundantQualifier
        ::Heuristics::HeuristicProvider<X, Y, Z, W> m_HeuristicProvider;
        Statistics::ScoreStatistics m_ScoreStatistics{};
        Task::LocalSearchTask<X, Y, Z, W>* mp_Task;
    };
}

#endif //LOCALSEARCH_H

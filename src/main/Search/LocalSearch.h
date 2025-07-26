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
    enum class LocalSearchType { LAHC, DLAS };
    
    template<typename X, typename Y, typename Z, typename W>
    class LocalSearch {
    public:
        // ReSharper disable CppRedundantQualifier
        explicit LocalSearch(const ::State::State<X, Y, Z, W> *initialState,
                             const std::vector<::Constraints::Constraint<X, Y, Z, W> *> &constraints,
                             LocalSearchType type = LocalSearchType::DLAS) noexcept :
            mp_InitialState(initialState),
            m_Constraints(constraints),
            m_HeuristicProvider(::Heuristics::HeuristicProvider<X, Y, Z, W>(initialState, constraints)) {
            switch (type) {
                case LocalSearchType::LAHC:
                    mp_Task = new Task::LahcLocalSearchTask<X, Y, Z, W>(*initialState, m_Constraints, m_ScoreStatistics);
                    break;
                case LocalSearchType::DLAS:
                    mp_Task = new Task::DlasLocalSearchTask<X, Y, Z, W>(*initialState, m_Constraints, m_ScoreStatistics);
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
            m_StartTime = std::chrono::time_point_cast<INSTANT_PRECISION>(std::chrono::system_clock::now());
            m_ScoreStatistics.startRecording(mp_Task->getInitialScore());
            m_CountedSteps = 0;
            m_StepCountTimePoint = duration_cast<std::chrono::milliseconds>(m_StartTime.time_since_epoch());
            m_StepsPerSecond = 0;
            m_AverageStepsPerSecond = 0;
            m_StepBatchCount = 0;
        }

        void endStatistics() noexcept {
            m_ScoreStatistics.finishRecording();
        }

        /**
         * @return `true` if new best state is found, `false` otherwise
         */
        bool step() noexcept {
            using namespace std::chrono_literals;

            // Finalizer
            if (mp_Task->shouldStep()) [[likely]] {
                mp_Task->step(m_HeuristicProvider);

                m_CountedSteps += 1;
                const auto currentTimePoint = std::chrono::system_clock::now();
                const auto currentTime = duration_cast<std::chrono::milliseconds>(currentTimePoint.time_since_epoch());
                if (const auto delta = currentTime - m_StepCountTimePoint; delta >= 1s) {
                    m_StepsPerSecond = m_CountedSteps * 1000 / (delta / 1ms);
                    m_CountedSteps = 0;
                    m_StepCountTimePoint = currentTime;
                    m_StepBatchCount += 1;
                    m_AverageStepsPerSecond += (static_cast<int64_t>(m_StepsPerSecond) - static_cast<int64_t>(m_AverageStepsPerSecond)) / static_cast<int64_t>(m_StepBatchCount);
                    const auto elapsedSeconds = (currentTimePoint.time_since_epoch() - m_StartTime.time_since_epoch()) / 1s;
                    std::cout << "States per second: " << m_StepsPerSecond << "; average: " << m_AverageStepsPerSecond << "; elapsed time: " << (elapsedSeconds / 60) << "m " << (elapsedSeconds % 60) << "s" << std::endl;
                }

                return mp_Task->newBestFound();
                // ReSharper disable once CppRedundantElseKeywordInsideCompoundStatement
            } else {
                m_Done = true;
                // printBestScore();
                return false;
            }
        }

        [[nodiscard]] bool isDone() const noexcept { return m_Done; }

        // ReSharper disable once CppRedundantQualifier
        ::State::State<X, Y, Z, W> getBestState() const noexcept { return mp_Task->getOutputState(); }
        [[nodiscard]] Score::Score getBestScore() const noexcept { return mp_Task->getOutputScore(); }

        [[nodiscard]] Score::Score evaluateCurrentBestState() const noexcept {
            Score::Score score {};
            // ReSharper disable once CppRedundantQualifier
            for (::Constraints::Constraint<X, Y, Z, W> *constraint : m_Constraints)
                score += constraint->evaluate(mp_Task->getOutputState());
            return score;
        }

        void printBestScore() const noexcept {
            std::cout << "Best score: " << mp_Task->getOutputScore() << "; delta=" << (mp_Task->getOutputScore() - mp_Task->m_InitScore) << std::endl;
        }

    protected:

    private:
        bool m_Done = false;
        Time::Instant m_StartTime{};
        uint64_t m_CountedSteps = 0;
        std::chrono::duration<int64_t, std::ratio<1, 1000>> m_StepCountTimePoint{};
        uint64_t m_StepsPerSecond = 0;
        uint64_t m_AverageStepsPerSecond = 0;
        uint64_t m_StepBatchCount = 0;

        // ReSharper disable once CppRedundantQualifier
        const ::State::State<X, Y, Z, W> *mp_InitialState;
        // ReSharper disable once CppRedundantQualifier
        const std::vector<::Constraints::Constraint<X, Y, Z, W> *> m_Constraints;
        // ReSharper disable once CppRedundantQualifier
        ::Heuristics::HeuristicProvider<X, Y, Z, W> m_HeuristicProvider;
        Statistics::ScoreStatistics m_ScoreStatistics{};
        Task::LocalSearchTask<X, Y, Z, W>* mp_Task;
    };
}

#endif //LOCALSEARCH_H

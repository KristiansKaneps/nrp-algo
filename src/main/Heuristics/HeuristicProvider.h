#ifndef HEURISTICPROVIDER_H
#define HEURISTICPROVIDER_H

#include "Moves/Perturbator.h"
#include "Moves/AutonomousPerturbator.h"
#include "Moves/PerturbatorChain.h"
#include "Search/Evaluation.h"
#include "State/State.h"

#include "Utils/Random.h"

#include "HyperHeuristics/TransformerModel.h"

#include "Moves/AssignPerturbator.h"
#include "Moves/HorizontalExchangePerturbator.h"
#include "Moves/UnassignPerturbator.h"
#include "Moves/RandomAssignmentTogglePerturbator.h"
#include "Moves/RankedIntersectionTogglePerturbator.h"
#include "Moves/ShiftByZPerturbator.h"
#include "Moves/VerticalExchangePerturbator.h"

#define HYPERHEURISTICS_HEURISTIC_COUNT 2
#define HYPERHEURISTICS_INPUT_DIM 9
#define HYPERHEURISTICS_BATCH_SIZE 1
#define HYPERHEURISTICS_SEQ_LENGTH 5
#define HYPERHEURISTICS_D_MODEL 32
#define HYPERHEURISTICS_N_HEAD 4
#define HYPERHEURISTICS_NUM_LAYERS 2

#define encodeViolationIntoTensor(i, violation) \
    tensor[0][i][0] = i; \
    tensor[0][i][1] = violation.score.strict; \
    tensor[0][i][2] = violation.score.hard; \
    tensor[0][i][3] = violation.score.soft; \
    tensor[0][i][4] = violation.getX(); \
    tensor[0][i][5] = violation.getY(); \
    tensor[0][i][6] = violation.getZ(); \
    tensor[0][i][7] = violation.getW(); \
    tensor[0][i][8] = violation.flags;

namespace Heuristics {
    using namespace ::Moves;

    template<typename X, typename Y, typename Z, typename W>
    class HeuristicProvider {
    public:
        explicit HeuristicProvider(const ::State::State<X, Y, Z, W> *initialState, const std::vector<::Constraints::Constraint<X, Y, Z, W> *>& constraints) noexcept :
            m_ConstraintCount(constraints.size()),
            m_TransformerModel(HYPERHEURISTICS_INPUT_DIM, HYPERHEURISTICS_D_MODEL, HYPERHEURISTICS_N_HEAD,
                               HYPERHEURISTICS_NUM_LAYERS, HYPERHEURISTICS_HEURISTIC_COUNT) {
            m_GeneratedPerturbators.shrink_to_fit();

            m_AvailablePerturbators = {
                new RandomAssignmentTogglePerturbator<X, Y, Z, W>(),
                new VerticalExchangePerturbator<X, Y, Z, W>(),
                new HorizontalExchangePerturbator<X, Y, Z, W>(),
                new ShiftByZPerturbator<X, Y, Z, W>(),
                new RankedIntersectionTogglePerturbator<X, Y, Z, W>(constraints),
            };

            torch::manual_seed(42);
        }

        ~HeuristicProvider() noexcept {
            for (const auto* peturbator : m_AvailablePerturbators)
                delete peturbator;
        }

        Perturbator<X, Y, Z, W> *operator[](const size_t index) const noexcept { return m_AvailablePerturbators[index]; }

        PerturbatorChain<X, Y, Z, W> predictPerturbators(const Evaluation::Evaluator<X, Y, Z, W>& evaluator,
                                                         const ::State::State<X, Y, Z, W>& state) noexcept {
            m_GeneratedPerturbators.clear();
            if (evaluator.m_TotalConstraintViolationCount == 0) return PerturbatorChain(m_GeneratedPerturbators);

            auto tensor = createInputTensor(evaluator);
            tensor = m_TransformerModel->forward(tensor);
            auto perturb = createHeuristicFromTensor(tensor, state.size());
            if (perturb == nullptr) return PerturbatorChain(m_GeneratedPerturbators);

            perturb->configure(state);
            if (perturb->isIdentity()) {
                delete perturb;
                return PerturbatorChain(m_GeneratedPerturbators);
            }

            m_GeneratedPerturbators.reserve(1);
            m_GeneratedPerturbators.emplace_back(perturb);

            return PerturbatorChain(m_GeneratedPerturbators);
        }

        [[nodiscard]] PerturbatorChain<X, Y, Z, W> generateRepairPerturbators(
            const Evaluation::Evaluator<X, Y, Z, W>& evaluator,
            const ::State::State<X, Y, Z, W>& state) noexcept {
            m_GeneratedPerturbators.clear();
            m_GeneratedPerturbators.reserve(evaluator.m_ViolatedConstraintCount);
            for (size_t i = 0; i < evaluator.m_ConstraintScores.size(); ++i) {
                const auto& constraint = evaluator.m_Constraints[i];
                const auto& constraintScore = evaluator.m_ConstraintScores[i];
                if (constraint->getRepairPerturbators().size() > 0) [[likely]] {
                    for (const auto& repairPerturbator : constraint->getRepairPerturbators()) {
                        for (const auto& violation : constraintScore.violations()) {
                            AutonomousPerturbator<X, Y, Z, W> *perturb = repairPerturbator->clone();
                            perturb->configure(&violation, state);
                            if (perturb->isIdentity()) {
                                delete perturb;
                                continue;
                            }
                            m_GeneratedPerturbators.emplace_back(perturb);
                        }
                    }
                }
            }
            return PerturbatorChain(m_GeneratedPerturbators);
        }

        [[nodiscard]] PerturbatorChain<X, Y, Z, W> generateSearchPerturbators(
            const Evaluation::Evaluator<X, Y, Z, W>& evaluator,
            const ::State::State<X, Y, Z, W>& state) noexcept {
            m_GeneratedPerturbators.clear();
            
            size_t count = m_Random.randomInt(1, 2);
            m_GeneratedPerturbators.reserve(count);

            // Use simpler heuristics more frequently (80% of time)
            if (m_Random.randomInt(0, 10) > 2) {
                // Fast random perturbations (no constraint analysis needed)
                const AutonomousPerturbator<X, Y, Z, W> *perturbTemplate = m_AvailablePerturbators[0];
                for (size_t i = 0; i < count; ++i) {
                    AutonomousPerturbator<X, Y, Z, W> *perturb = perturbTemplate->clone();
                    perturb->configure(nullptr, state);
                    if (!perturb->isIdentity()) {
                        m_GeneratedPerturbators.emplace_back(perturb);
                    } else {
                        delete perturb;
                    }
                }
            } else {
                // Smarter heuristics (20% of time)
                for (const auto* perturbTemplate : m_AvailablePerturbators) {
                    if (m_GeneratedPerturbators.size() >= count) break;
                    auto* perturb = perturbTemplate->clone();
                    if (perturb->configureIfApplicable(evaluator, state) && !perturb->isIdentity()) {
                        m_GeneratedPerturbators.emplace_back(perturb);
                    } else {
                        delete perturb;
                    }
                }
            }

            return PerturbatorChain(m_GeneratedPerturbators);
        }

    private:
        inline static Random::RandomGenerator& m_Random = Random::generator();

        const size_t m_ConstraintCount;
        std::vector<AutonomousPerturbator<X, Y, Z, W> *> m_AvailablePerturbators;
        HyperHeuristics::TransformerModel m_TransformerModel;

        std::vector<Perturbator<X, Y, Z, W> *> m_GeneratedPerturbators {};

        [[nodiscard]] torch::Tensor createInputTensor(const Evaluation::Evaluator<X, Y, Z, W>& evaluator) noexcept {
            constexpr int batchSize = 1;
            const int seqLength = evaluator.m_TotalConstraintViolationCount;
            constexpr int featureSize = 9;

            torch::Tensor tensor = torch::zeros({batchSize, seqLength, featureSize}, torch::kFloat32);

            for (size_t i = 0; i < evaluator.m_ConstraintScores.size(); ++i) {
                for (const auto& constraintScore = evaluator.m_ConstraintScores[i]; auto& violation : constraintScore.violations()) {
                    encodeViolationIntoTensor(i, violation);
                }
            }

            return tensor;
        }

        [[nodiscard]] static Perturbator<X, Y, Z, W>* createHeuristicFromTensor(const torch::Tensor& tensor, const ::State::Size& stateSize) noexcept {
            const size_t heuristicIndex = static_cast<size_t>(tensor[0].argmax().item<int>()); // The first few entries are scores per heuristic
            const axis_size_t x = static_cast<axis_size_t>(tensor[1].item<float>());
            const axis_size_t y = static_cast<axis_size_t>(tensor[2].item<float>());
            const axis_size_t z = static_cast<axis_size_t>(tensor[3].item<float>());
            const axis_size_t w = static_cast<axis_size_t>(tensor[4].item<float>());

            if (heuristicIndex >= HYPERHEURISTICS_HEURISTIC_COUNT || x >= stateSize.width || y >= stateSize.height || z >= stateSize.depth || w >= stateSize.concepts) {
                return nullptr;
            }

            switch (heuristicIndex) {
                case 0:
                    return new UnassignPerturbator<X, Y, Z, W>(::State::Location{x, y, z, w});
                case 1:
                    return new AssignPerturbator<X, Y, Z, W>(::State::Location{x, y, z, w});
                default:
                    return nullptr;
            }
        }
    };
}

#endif //HEURISTICPROVIDER_H

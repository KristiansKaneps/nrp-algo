#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include <string>
#include <utility>

#include "ConstraintScore.h"
#include "Moves/AutonomousPerturbator.h"
#include "State/State.h"

namespace Constraints {
    template<typename X, typename Y, typename Z, typename W>
    class Constraint {
    public:
        explicit Constraint(std::string name,
                            const std::vector<Moves::AutonomousPerturbator<X, Y, Z, W> *> repairPerturbators) noexcept :
            m_Name(std::move(name)),
            m_RepairPerturbators(std::move(repairPerturbators)) { }

        virtual ~Constraint() noexcept {
            for (const Moves::AutonomousPerturbator<X, Y, Z, W> *repairPerturbator : m_RepairPerturbators)
                delete repairPerturbator;
        }

        Constraint(const Constraint&) noexcept = default;

        [[nodiscard]] virtual bool printsInfo() const noexcept { return false; }
        virtual void printInfo() const noexcept { }

        [[nodiscard]] const std::string& name() const noexcept { return m_Name; }

        [[nodiscard]] const std::vector<Moves::AutonomousPerturbator<X, Y, Z, W> *>& getRepairPerturbators() const noexcept { return m_RepairPerturbators; }

        virtual ConstraintScore evaluate(const ::State::State<X, Y, Z, W>& state) noexcept = 0;

    private:
        std::string m_Name;
        const std::vector<Moves::AutonomousPerturbator<X, Y, Z, W> *> m_RepairPerturbators;
    };
}

template<typename X, typename Y, typename Z, typename W>
struct std::hash<Constraints::Constraint<X, Y, Z, W>> { // NOLINT(*-dcl58-cpp)
    std::size_t operator()(const Constraints::Constraint<X, Y, Z, W>& k) const noexcept {
        using std::size_t;
        using std::hash;
        using std::string;
        return hash<uint64_t>()(k.name());
    }
};

#endif //CONSTRAINT_H

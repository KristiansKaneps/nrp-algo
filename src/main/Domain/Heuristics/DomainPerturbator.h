#ifndef DOMAINPERTURBATOR_H
#define DOMAINPERTURBATOR_H

#include "Heuristics/Perturbator.h"
#include "Heuristics/AutonomousPerturbator.h"

#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Employee.h"
#include "Domain/Entities/Day.h"
#include "Domain/Entities/Skill.h"

#include "Heuristics/Default/UnassignRepairPerturbator.h"
#include "Heuristics/Default/HorizontalExchangePerturbator.h"
#include "Heuristics/Default/VerticalExchangePerturbator.h"

namespace Domain::Heuristics {
    using DomainPerturbator = ::Heuristics::Perturbator<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>;
    using DomainAutonomousPerturbator = ::Heuristics::AutonomousPerturbator<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>;
    using DomainIdentityPerturbator = ::Heuristics::IdentityPerturbator<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>;

    // Default
    using DomainUnassignRepairPerturbator = ::Heuristics::UnassignRepairPerturbator<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>;
    using DomainHorizontalExchangePerturbator = ::Heuristics::HorizontalExchangePerturbator<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>;
    using DomainVerticalExchangePerturbator = ::Heuristics::VerticalExchangePerturbator<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>;
}

#endif //DOMAINPERTURBATOR_H

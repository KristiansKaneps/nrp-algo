#ifndef DOMAINPERTURBATOR_H
#define DOMAINPERTURBATOR_H

#include "Moves/Perturbator.h"
#include "Moves/AutonomousPerturbator.h"

#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Employee.h"
#include "Domain/Entities/Day.h"
#include "Domain/Entities/Skill.h"

#include "Moves/UnassignRepairPerturbator.h"
#include "Moves/HorizontalExchangePerturbator.h"
#include "Moves/VerticalExchangePerturbator.h"

namespace Domain::Moves {
    using DomainPerturbator = ::Moves::Perturbator<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>;
    using DomainAutonomousPerturbator = ::Moves::AutonomousPerturbator<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>;
    using DomainIdentityPerturbator = ::Moves::IdentityPerturbator<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>;

    // Default
    using DomainUnassignRepairPerturbator = ::Moves::UnassignRepairPerturbator<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>;
    using DomainHorizontalExchangePerturbator = ::Moves::HorizontalExchangePerturbator<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>;
    using DomainVerticalExchangePerturbator = ::Moves::VerticalExchangePerturbator<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>;
}

#endif //DOMAINPERTURBATOR_H

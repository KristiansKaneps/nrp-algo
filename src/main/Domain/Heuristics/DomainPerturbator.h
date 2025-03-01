#ifndef DOMAINPERTURBATOR_H
#define DOMAINPERTURBATOR_H

#include "Heuristics/Perturbator.h"

#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Employee.h"
#include "Domain/Entities/Day.h"
#include "Domain/Entities/Skill.h"

namespace Domain::Heuristics {
    using DomainPerturbator = ::Heuristics::Perturbator<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>;
    using DomainIdentityPerturbator = ::Heuristics::IdentityPerturbator<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>;
}

#endif //DOMAINPERTURBATOR_H

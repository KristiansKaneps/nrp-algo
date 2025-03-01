#ifndef DOMAINCONSTRAINT_H
#define DOMAINCONSTRAINT_H

#include "Constraints/Constraint.h"

#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Employee.h"
#include "Domain/Entities/Day.h"
#include "Domain/Entities/Skill.h"

namespace Domain::Constraints {
    using DomainConstraint = ::Constraints::Constraint<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>;

    using axis_size_t = State::axis_size_t;
    using state_size_t = State::state_size_t;
    using score_t = Score::score_t;
}

#endif //DOMAINCONSTRAINT_H

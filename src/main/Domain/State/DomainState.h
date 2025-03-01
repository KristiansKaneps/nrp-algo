#ifndef DOMAINSTATE_H
#define DOMAINSTATE_H

#include "State/State.h"

#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Employee.h"
#include "Domain/Entities/Day.h"
#include "Domain/Entities/Skill.h"

namespace Domain::State {
    using DomainState = ::State::State<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>;
}

#endif //DOMAINSTATE_H

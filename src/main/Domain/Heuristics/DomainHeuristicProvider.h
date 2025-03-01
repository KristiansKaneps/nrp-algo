#ifndef DOMAINHEURISTICPROVIDER_H
#define DOMAINHEURISTICPROVIDER_H

#include "Heuristics/HeuristicProvider.h"

#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Employee.h"
#include "Domain/Entities/Day.h"
#include "Domain/Entities/Skill.h"

namespace Domain::Heuristics {
    using DomainHeuristicProvider = ::Heuristics::HeuristicProvider<Domain::Shift, Domain::Employee, Domain::Day, Domain::Skill>;
}

#endif //DOMAINHEURISTICPROVIDER_H

#include "Example.h"

#if EXAMPLE == 4

#include "NrpProblemInstances/NrpProblemInstanceParser.h"

#include "Domain/Constraints/EmployeeGeneralConstraint.h"
#include "Domain/Constraints/ValidShiftDayConstraint.h"
#include "Domain/Constraints/NoOverlapConstraint.h"
#include "Domain/Constraints/RequiredSkillConstraint.h"
#include "Domain/Constraints/ShiftCoverageConstraint.h"
#include "Domain/Constraints/EmploymentMaxDurationConstraint.h"
#include "Domain/Constraints/RestBetweenShiftsConstraint.h"
#include "Domain/Constraints/EmployeeAvailabilityConstraint.h"
#include "Domain/Constraints/CumulativeFatigueConstraint.h"

#include "Domain/Heuristics/DomainHeuristicProvider.h"

#include "Search/LocalSearch.h"

using namespace Domain;

void Example::create() {

    NrpProblemInstances::NrpProblemInstanceParser parser("Instance1.txt");
    parser.parse();

    const std::chrono::time_zone *const timeZone = parser.timeZone();
    const Time::Range range = parser.range();

    const uint32_t shiftCount = parser.shiftCount();
    const uint32_t employeeCount = parser.employeeCount();
    const uint32_t dayCount = range.getDayCount(timeZone);
    const uint32_t skillCount = parser.skillCount();

    std::cout << "Range workload duration is " << (dayCount * 8) << "h" << std::endl;
    std::cout << "Max workload duration (workdays * 8h) is " << (range.getWorkdayCount(timeZone) * 8) << "h" << std::endl;

    std::allocator<Day> dayAllocator;
    std::allocator<Employee> employeeAllocator;
    std::allocator<Shift> shiftAllocator;
    std::allocator<Skill> skillAllocator;

    auto *shifts = shiftAllocator.allocate(shiftCount);
    auto *employees = employeeAllocator.allocate(employeeCount);
    auto *days = dayAllocator.allocate(dayCount);
    auto *skills = skillAllocator.allocate(skillCount);

    for (axis_size_t i = 0; i < shiftCount; ++i) { new(shifts + i) Shift(parser.shifts()[i]); }
    for (uint32_t i = 0; i < employeeCount; ++i) { new(employees + i) Employee(parser.employees()[i]); }
    for (uint32_t i = 0; i < dayCount; ++i) { new(days + i) Day(i, range.getDayRangeAt(i, std::chrono::current_zone())); }
    for (uint32_t i = 0; i < skillCount; ++i) { new(skills + i) Skill(parser.skills()[i]); }

    auto *axisX = new Axes::Axis(shifts, shiftCount);
    auto *axisY = new Axes::Axis(employees, employeeCount);
    auto *axisZ = new Axes::Axis(days, dayCount);
    auto *axisW = new Axes::Axis(skills, skillCount);

    Domain::State::DomainState state(
        range,
        timeZone,
        axisX,
        axisY,
        axisZ,
        axisW
    );

    // for (int i = 0; i < shiftCount; i++) { state.set(i, 0, 0, 0); }
    // for (int i = 0; i < dayCount; i++) { state.set(3, 2, i, 1); }
    // for (int i = 0; i < dayCount; i++) { state.set(2, 2, i, 1); }

    // state.random(0.2);

    // state.set(0, 0, 0, 0);
    // state.set(1, 1, 1, 3);

    state.printSize();

    auto *employeeGeneralConstraint = new Domain::Constraints::EmployeeGeneralConstraint(state.range(), state.timeZone(), state.z());
    auto *validShiftDayConstraint = new Domain::Constraints::ValidShiftDayConstraint(state.range(), state.timeZone(), state.x(), state.y().size(), state.z(), state.w().size());
    auto *noOverlapConstraint = new Domain::Constraints::NoOverlapConstraint(state.x());
    auto *requiredSkillConstraint = new Domain::Constraints::RequiredSkillConstraint(state.x(), state.y(), state.w());
    auto *shiftCoverageConstraint = new Domain::Constraints::ShiftCoverageConstraint(state.range(), state.timeZone(), state.x(), state.z());
    auto *employmentDurationConstraint = new Domain::Constraints::EmploymentMaxDurationConstraint(state.range(), 7, state.timeZone(), state.x(), state.y(), state.z());
    auto *restBetweenShiftsConstraint = new Domain::Constraints::RestBetweenShiftsConstraint(state.x());
    auto *employeeAvailabilityConstraint = new Domain::Constraints::EmployeeAvailabilityConstraint(state.range(), state.timeZone(), state.x(), state.y(), state.z());
    auto *cumulativeFatigueConstraint = new Domain::Constraints::CumulativeFatigueConstraint(state.x());

    const auto constraints = std::vector<::Constraints::Constraint<Shift, Employee, Day, Skill> *> {
        employeeGeneralConstraint,
        validShiftDayConstraint,
        noOverlapConstraint,
        requiredSkillConstraint,
        shiftCoverageConstraint,
        employmentDurationConstraint,
        restBetweenShiftsConstraint,
        employeeAvailabilityConstraint,
        cumulativeFatigueConstraint,
    };

    const auto heuristicProvider = Domain::Heuristics::DomainHeuristicProvider(constraints.size());

    std::cout << "State 1 score: " << Evaluation::evaluateState(state, constraints) << std::endl;
    state.clearAll();
    // state.random(0.025);

    // ReSharper disable CppDFANullDereference
    gp_AppState = new AppState{
        .score = Evaluation::evaluateState(state, constraints),
        .state = state,
        .constraints = constraints,
        .heuristicProvider = heuristicProvider,
    };

    gp_Update = new LocalSearchUpdate{
        .state = gp_AppState->state,
        .score = gp_AppState->score,
    };
    // ReSharper restore CppDFANullDereference
}

#endif
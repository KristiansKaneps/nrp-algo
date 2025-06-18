#include "Example.h"

#if EXAMPLE == 3

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
#include "Domain/Heuristics/RandomAssignmentTogglePerturbator.h"

#include "Search/LocalSearch.h"

using namespace Domain;

void Example::create() {
    std::tm tm = {};
    std::stringstream ss("2024-12-01T00:00:00Z");
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
    const auto start = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    ss = std::stringstream("2025-01-01T00:00:00Z");
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
    const auto end = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    const std::chrono::time_zone *const timeZone = std::chrono::current_zone();
    const Time::Range range(start, end);

    const uint32_t shiftCount = 2;
    const uint32_t employeeCount = 12;
    const uint32_t dayCount = range.getDayCount(timeZone);
    const uint32_t skillCount = 1;

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

    const Time::DailyInterval interval1("08:00", 480);
    const Time::DailyInterval interval2("16:00", 480);
    // const Time::DailyInterval interval3("08:00", 1440);
    // const Time::DailyInterval interval4("20:00", 1440);

    new(shifts + 0) Shift(0, Shift::ALL_WEEKDAYS, interval1, "E", 4);
    new(shifts + 1) Shift(1, Shift::ALL_WEEKDAYS, interval2, "L", 4);
    // new(shifts + 2) Shift(2, Shift::ONLY_WEEKENDS, interval3, "DN", 1, static_cast<Time::day_minutes_t>(2 * interval3.durationInMinutes()));
    // new(shifts + 3) Shift(3, Shift::ONLY_WEEKENDS, interval4, "ND", 1, static_cast<Time::day_minutes_t>(2 * interval4.durationInMinutes()));

    for (uint32_t i = 0; i < employeeCount; ++i) { new(employees + i) Employee(i); }

    for (uint32_t i = 0; i < dayCount; ++i) {
        new(days + i) Day(i, range.getDayRangeAt(i, std::chrono::current_zone()));
        // std::cout << days[i].range().start() << ' ' << days[i].range().end() << std::endl;
    }

    for (uint32_t i = 0; i < skillCount; ++i) { new(skills + i) Skill(i, std::to_string(i)); }

    for (size_t i = 0; i < employeeCount; ++i) {
        auto &employee = employees[i];
        employee.addSkill(0, {1.0f, Workload::Strategy::STATIC, {0.0f, 1.0f, 0.0f}});
    }

    for (size_t i = 0; i < shiftCount; ++i) {
        shifts[i].addRequiredAllSkill(0, 1.0f);
    }

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

    auto heuristic1 = new Domain::Heuristics::RandomAssignmentTogglePerturbator();

    const auto heuristicProvider = Domain::Heuristics::DomainHeuristicProvider(constraints.size(), {
        heuristic1,
    });

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
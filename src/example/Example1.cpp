#include "Example.h"

#if EXAMPLE == 1

#include "Domain/Constraints/NoOverlapConstraint.h"
#include "Domain/Constraints/RequiredSkillConstraint.h"
#include "Domain/Constraints/ShiftCoverageConstraint.h"
#include "Domain/Constraints/EmploymentMaxDurationConstraint.h"
#include "Domain/Constraints/RestBetweenShiftsConstraint.h"
#include "Domain/Constraints/EmployeeAvailabilityConstraint.h"

#include "Search/LocalSearch.h"

#include "Time/RangeCollection.h"

#include "State/State.h"

using namespace Domain;

void Example::create() {
    std::tm tm = {};
    std::stringstream ss("2024-12-01T00:00:00Z");
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
    auto start = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    ss = std::stringstream("2025-01-01T00:00:00Z");
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
    auto end = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    const std::chrono::time_zone *const timeZone = std::chrono::current_zone();
    const Time::Range range(start, end);

    const uint32_t shiftCount = 4;
    const uint32_t employeeCount = 30;
    const uint32_t dayCount = 31;
    const uint32_t skillCount = 4;

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
    const Time::DailyInterval interval3("08:00", 1440);
    const Time::DailyInterval interval4("20:00", 1440);

    new(shifts + 0) Shift(0, interval1, "E", 2);
    new(shifts + 1) Shift(1, interval2, "L", 2);
    new(shifts + 2) Shift(2, interval3, "DN", 2);
    new(shifts + 3) Shift(3, interval4, "ND", 2);

    for (uint32_t i = 0; i < employeeCount; ++i) { new(employees + i) Employee(i); }

    for (uint32_t i = 0; i < dayCount; ++i) {
        new(days + i) Day(i, range.getDayRangeAt(i, std::chrono::current_zone()));
        // std::cout << days[i].range().start() << ' ' << days[i].range().end() << std::endl;
    }

    for (uint32_t i = 0; i < skillCount; ++i) { new(skills + i) Skill(i, std::to_string(i)); }

    employees[2].addSkill(0, {1.0f, Workload::Strategy::STATIC, {0.0f, 1.0f, 0.0f}});
    employees[2].addSkill(3, {1.0f, Workload::Strategy::STATIC, {0.0f, 1.0f, 0.0f}});
    employees[2].addSkill(1, {1.0f, Workload::Strategy::STATIC, {0.0f, 1.0f, 0.0f}});

    shifts[0].addRequiredAllSkill(0, 1.0f);
    shifts[0].addRequiredOneSkill(1, 1.0f);
    shifts[0].addRequiredOneSkill(2, 1.0f);

    shifts[1].addRequiredAllSkill(1, 1.0f);

    shifts[2].addRequiredAllSkill(2, 1.0f);

    shifts[3].addRequiredAllSkill(3, 1.0f);
    shifts[3].addRequiredAllSkill(0, 0.5f);
    shifts[3].addRequiredAllSkill(1, 1.0f);
    shifts[3].addRequiredOneSkill(0, 1.0f);
    shifts[3].addRequiredOneSkill(1, 1.0f);

    auto *axisX = new Axes::Axis(shifts, shiftCount);
    auto *axisY = new Axes::Axis(employees, employeeCount);
    auto *axisZ = new Axes::Axis(days, dayCount);
    auto *axisW = new Axes::Axis(skills, skillCount);

    State::State<Shift, Employee, Day, Skill> state(
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

    auto *noOverlapConstraint = new Constraints::NoOverlapConstraint(state.x());
    auto *requiredSkillConstraint = new Constraints::RequiredSkillConstraint(state.x(), state.y(), state.w());
    auto *shiftCoverageConstraint = new Constraints::ShiftCoverageConstraint(state.range(), state.timeZone(), state.x(), state.z());
    auto *employmentDurationConstraint = new Constraints::EmploymentMaxDurationConstraint(state.range(), state.timeZone(), state.x(), state.y(), state.z());
    auto *restBetweenShiftsConstraint = new Constraints::RestBetweenShiftsConstraint(state.x());
    auto *employeeAvailabilityConstraint = new Constraints::EmployeeAvailabilityConstraint(state.range(), state.timeZone(), state.x(), state.y(), state.z());

    const auto constraints = std::vector<Constraints::Constraint<Shift, Employee, Day, Skill> *> {
        noOverlapConstraint,
        requiredSkillConstraint,
        shiftCoverageConstraint,
        employmentDurationConstraint,
        restBetweenShiftsConstraint,
        employeeAvailabilityConstraint,
    };

    std::cout << "State 1 score: " << Evaluation::evaluateState(state, constraints) << std::endl;
    state.clearAll();
    // state.random(0.025);

    // ReSharper disable CppDFANullDereference
    gp_AppState = new AppState{
        .score = Evaluation::evaluateState(state, constraints),
        .state = state,
        .constraints = constraints,
    };

    gp_Update = new LocalSearchUpdate{
        .state = gp_AppState->state,
        .score = gp_AppState->score,
    };
    // ReSharper restore CppDFANullDereference
}

#endif
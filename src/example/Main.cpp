#include "Application.h"
#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Employee.h"
#include "Domain/Entities/Day.h"
#include "Domain/Entities/Skill.h"

#include "Domain/Constraints/NoOverlapConstraint.h"
#include "Domain/Constraints/RequiredSkillConstraint.h"

#include "Search/LocalSearch.h"
#include "State/State.h"

#include "Time/RangeCollection.h"

using namespace Domain;

int main(int argc, char **argv) {
    std::tm tm = {};
    std::stringstream ss("2024-12-01T00:00:00Z");
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
    auto start = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    ss = std::stringstream("2025-01-01T00:00:00Z");
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
    auto end = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    const Time::Range range(start, end);

    const uint32_t shiftCount = 4;
    const uint32_t employeeCount = 50;
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

    const Time::DailyInterval interval1(480, 480);
    const Time::DailyInterval interval2(960, 480);
    const Time::DailyInterval interval3(480, 1440);
    const Time::DailyInterval interval4(1200, 1440);

    new(shifts + 0) Shift(0, interval1, "E");
    new(shifts + 1) Shift(1, interval2, "L");
    new(shifts + 2) Shift(2, interval3, "DN");
    new(shifts + 3) Shift(3, interval4, "ND");

    for (uint32_t i = 0; i < employeeCount; ++i) { new(employees + i) Employee(i); }

    for (uint32_t i = 0; i < dayCount; ++i) {
        new(days + i) Day(i, range.getDayRangeAt(i, std::chrono::current_zone()));
        // std::cout << days[i].range().start() << ' ' << days[i].range().end() << std::endl;
    }

    for (uint32_t i = 0; i < skillCount; ++i) { new(skills + i) Skill(i, std::to_string(i)); }

    employees[2].addSkill(0, 1.0f);
    employees[2].addSkill(3, 1.0f);
    employees[2].addSkill(1, 1.0f);

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

    State::State state(
        range,
        Axes::Axis(shifts, shiftCount),
        Axes::Axis(employees, employeeCount),
        Axes::Axis(days, dayCount),
        Axes::Axis(skills, skillCount)
    );

    // for (int i = 0; i < shiftCount; i++) { state.set(i, 0, 0, 0); }
    for (int i = 0; i < dayCount; i++) { state.set(3, 2, i, 1); }

    // state.random();

    state.printSize();

    Constraints::NoOverlapConstraint noOverlapConstraint(state.x());
    Constraints::RequiredSkillConstraint requiredSkillConstraint(state.x(), state.y(), state.w());

    const auto constraints = std::vector<Constraints::Constraint<Shift, Employee, Day, Skill> *> {
        &noOverlapConstraint,
        &requiredSkillConstraint,
    };
    Search::LocalSearch localSearch(&state, constraints);
    const auto score = localSearch.evaluateCurrentState();

    std::cout << "Score: " << score.hard << ", " << score.soft << std::endl;









    const auto now = std::chrono::system_clock::now();
    const std::chrono::duration<int64_t, std::nano> second(1000000000);

    Time::Range range1(now, now + second);
    Time::Range range2(now + 2 * second, now + 4 * second);

    Time::RangeCollection collection(2);
    collection.add(range1);
    collection.add(range2);

    Time::Ray ray = now + second;
    std::cout << "Ray intersects range 1 & 2: " << (ray.intersects(range1) ? "true" : "false") << ' ' << (
        ray.intersects(range2) ? "true" : "false") << std::endl;
    std::cout << "Ray intersects range collection: " << (ray.intersects(collection) ? "true" : "false") << std::endl;

    // std::cout << "Press any key to exit..." << std::endl;
    // std::cin.get();
    // std::cout << "Exited." << std::endl;

    Application app(1280, 720, "Example Application");
    app.start();
}

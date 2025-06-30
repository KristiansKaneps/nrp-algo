#include "Example.h"

#if EXAMPLE == 2

#include "Domain/Constraints/EmployeeGeneralConstraint.h"
#include "Domain/Constraints/ValidShiftDayConstraint.h"
#include "Domain/Constraints/NoOverlapConstraint.h"
#include "Domain/Constraints/RequiredSkillConstraint.h"
#include "Domain/Constraints/ShiftCoverageConstraint.h"
#include "Domain/Constraints/EmploymentMaxDurationConstraint.h"
#include "Domain/Constraints/RestBetweenShiftsConstraint.h"
#include "Domain/Constraints/EmployeeAvailabilityConstraint.h"
#include "Domain/Constraints/CumulativeFatigueConstraint.h"

#include "Search/LocalSearch.h"

#include "Time/RangeCollection.h"

#include "Database.h"
#include "UserDataAccumulator.h"
#include "ShiftDataAccumulator.h"
#include "SkillDataAccumulator.h"

using namespace Domain;

void Example::create() {
    std::tm tm = {};
    std::stringstream ss("2025-03-01T00:00:00Z");
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
    auto start = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    ss = std::stringstream("2025-04-01T00:00:00Z");
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S%z");
    auto end = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    const std::chrono::time_zone *const timeZone = std::chrono::current_zone();
    const Time::Range range(start, end);

    std::allocator<Day> dayAllocator;
    std::allocator<Employee> employeeAllocator;
    std::allocator<Shift> shiftAllocator;
    std::allocator<Skill> skillAllocator;

    size_t shiftCount, employeeCount, dayCount, skillCount;

    Database database;

    if (!database.isConnected()) {
        throw std::runtime_error("Database not connected");
    }

    const std::string rangeStart = Time::InstantToString(range.start());
    const std::string rangeEnd = Time::InstantToString(range.end());

    // std::cout << "Range start as string: " << rangeStart << std::endl;
    // const auto parsedInstant = Time::StringToInstant(rangeStart);
    // std::cout << "Range start parsed: " << parsedInstant << std::endl;

    // BKUS organization ID = '79cf52f8-8c59-457e-9b2a-d5ec1f983776'
    // NMPON organizational unit ID = 'b2ecb1e5-0cf6-41e9-95cf-07a30dd14ebd'
    // Ārsti department ID = '5193c3e7-1a87-40a9-9ef6-f1acb276f00f'

    database.execute(
        "select s.id, s.name, s.complementary from skills s "
        "join department_skills ds on ds.skill_id = s.id "
        "where s.organization_id = '79cf52f8-8c59-457e-9b2a-d5ec1f983776' "
        "and (s.deleted_at is null or s.deleted_at > '" + rangeStart + "') "
        "and ds.department_id = '5193c3e7-1a87-40a9-9ef6-f1acb276f00f'"
    );

    SkillData::Accumulator skillAccumulator;

    for (uint32_t i = 0; i < database.tupleCount(); ++i) {
        const char *skillId = database.value(i, 0);
        const char *skillName = database.value(i, 1);
        const char *skillComplementaryStr = database.value(i, 2);
        const bool skillComplementary = skillComplementaryStr[0] == 't';
        skillAccumulator.addSkill(skillId, skillName, skillComplementary);
    }

    std::cout << "Total skill count: " << skillAccumulator.size() << std::endl;

    database.execute(
        "select u.id, u.name, u.surname, u.email, a.type, a.unavailability_subtype, a.start_date, a.end_date "
        "from users u join organizational_unit_users ouu on ouu.user_id = u.id and ouu.organizational_unit_id = 'b2ecb1e5-0cf6-41e9-95cf-07a30dd14ebd' "
        "left join availabilities a on a.user_id = u.id and a.start_date < '" + rangeEnd + "' and a.end_date > '" + rangeStart + "' "
        "where (u.deleted_at is null or u.deleted_at > '" + rangeStart + "') "
        "and (ouu.deleted_at is null or ouu.deleted_at > '" + rangeStart + "')"
    );

    // std::cout << "Tuple count: " << database.tupleCount() << std::endl;
    // std::cout << "Attribute count: " << database.attributeCount() << std::endl;
    // std::cout << "Error: " << database.errorMessage() << std::endl;

    UserData::Accumulator userAccumulator;

    for (uint32_t i = 0; i < database.tupleCount(); ++i) {
        const char *userId = database.value(i, 0);
        const char *userName = database.value(i, 1);
        const char *userSurname = database.value(i, 2);
        const char *userEmail = database.value(i, 3);
        const auto availabilityType = static_cast<UserData::AvailabilityType>(2 - atoi(database.value(i, 4))); // NOLINT(*-err34-c)
        const auto unavailabilitySubtype = static_cast<UserData::UnavailabilitySubtype>(atoi(database.value(i, 5))); // NOLINT(*-err34-c)

        userAccumulator.addUserFullName(userId, userEmail, userName, userSurname);

        if (!database.isNull(i, 6) && !database.isNull(i, 7)) {
            const char *availabilityRangeStartStr = database.value(i, 6);
            const char *availabilityRangeEndStr = database.value(i, 7);
            const auto availabilityRangeStart = Time::StringToInstant(availabilityRangeStartStr);
            const auto availabilityRangeEnd = Time::StringToInstant(availabilityRangeEndStr);
            const Time::Range availabilityRange(availabilityRangeStart, availabilityRangeEnd);
            userAccumulator.addAvailability(userId, availabilityRange, availabilityType, unavailabilitySubtype);
        }
    }

    std::cout << "Total user count: " << userAccumulator.size() << std::endl;

    std::unordered_map<std::string, std::unordered_map<std::string, Workload::ChangeEvent>> workloadChangeEvents;

    for (uint32_t i = 0; i < userAccumulator.size(); ++i) {
        const auto &userId = userAccumulator.id(i);
        database.execute(
            "select max(wce.instant), wce.skill_id "
            "from workload_change_events wce "
            "where wce.user_id = '" + userId + "' and wce.organizational_unit_id = 'b2ecb1e5-0cf6-41e9-95cf-07a30dd14ebd' "
            "and wce.instant <= '" + rangeStart + "' "
            "group by wce.skill_id, wce.dynamic_load_hours, wce.static_load, wce.max_overtime_hours"
        );

        if (!database.tupleCount()) {
            userAccumulator.removeUserById(userId);
            continue;
        }

        auto events = std::unordered_map<std::string, Workload::ChangeEvent>();

        for (uint32_t j = 0; j < database.tupleCount(); ++j) {
            const auto instantStr = std::string(database.value(0, 0));
            const auto skillId = std::string(database.value(0, 1));

            database.execute(
                "select wce.dynamic_load_hours, wce.static_load, wce.max_overtime_hours "
                "from workload_change_events wce "
                "where wce.organizational_unit_id = 'b2ecb1e5-0cf6-41e9-95cf-07a30dd14ebd' "
                "and wce.skill_id = '" + skillId + "' and wce.user_id = '" + userId + "' "
                "and wce.instant = '" + instantStr + "'"
            );
            assert(database.tupleCount() == 1 && "Workload change event tuple count must be 1");

            const char *dynamicLoadHoursStr = database.value(0, 0);
            const auto dynamicLoadHours = static_cast<float>(atof(dynamicLoadHoursStr)); // NOLINT(*-err34-c)
            const char *staticLoadStr = database.value(0, 1);
            const auto staticLoad = static_cast<float>(atof(staticLoadStr)); // NOLINT(*-err34-c)
            const char *maxOvertimeHoursStr = database.value(0, 2);
            const auto maxOvertimeHours = static_cast<float>(atof(maxOvertimeHoursStr)); // NOLINT(*-err34-c)

            events.insert({skillId, {dynamicLoadHours, staticLoad, maxOvertimeHours}});
        }

        workloadChangeEvents.insert({userId, events});
    }

    for (size_t i = 0; i < userAccumulator.size(); ++i) {
        const auto &userId = userAccumulator.id(i);

        const auto eventsIt = workloadChangeEvents.find(userId);
        if (eventsIt == workloadChangeEvents.end()) continue;
        const auto &events = eventsIt->second;

        database.execute(
            "select usk.skill_id, usk.weight, usk.strategy from user_skills usk "
            "join skills s on s.id = usk.skill_id "
            "where s.organization_id = '79cf52f8-8c59-457e-9b2a-d5ec1f983776' and usk.user_id = '" + userId + "' "
            "and (s.deleted_at is null or s.deleted_at > '" + rangeStart + "') "
            "and (usk.deleted_at is null or usk.deleted_at > '" + rangeStart + "') "
            "and usk.created_at < '" + rangeEnd + "' "
        );

        const size_t rowCount = database.tupleCount();
        std::vector<UserData::Skill> userSkills;

        for (uint32_t j = 0; j < rowCount; ++j) {
            const char *skillId = database.value(j, 0);
            const char *skillWeightStr = database.value(j, 1);
            const auto skillWeight = static_cast<float>(atof(skillWeightStr)); // NOLINT(*-err34-c)
            const char *skillStrategyStr = database.value(j, 2);
            const auto skillStrategy = static_cast<Domain::Workload::Strategy>(atoi(skillStrategyStr)); // NOLINT(*-err34-c)

            if (const auto wceIt = events.find(skillId); wceIt == events.end()) {
                userSkills.emplace_back(skillId, skillWeight, skillStrategy, Workload::ChangeEvent{});
            } else {
                userSkills.emplace_back(skillId, skillWeight, skillStrategy, wceIt->second);
            }
        }

        userAccumulator.addSkills(userId, userSkills);
    }

    const std::string shiftTableName = "shifts";
    const std::string shiftSkillsTableName = "shift_skills";

    database.execute(
        "select s.id, s.summary, s.weight, s.color, s.start_date, s.end_date, s.locked, s.user_id, "
        "s.assigned_skill_id "
        "from " + shiftTableName + " s "
        "where s.department_id = '5193c3e7-1a87-40a9-9ef6-f1acb276f00f' and "
        "s.start_date < '" + rangeEnd + "' and s.end_date > '" + rangeStart + '\''
    );

    ShiftData::Accumulator shiftAccumulator;

    for (uint32_t i = 0; i < database.tupleCount(); ++i) {
        const char *shiftId = database.value(i, 0);
        const char *shiftSummary = database.value(i, 1);
        const char *shiftWeightStr = database.value(i, 2);
        const auto shiftWeight = static_cast<float>(atof(shiftWeightStr)); // NOLINT(*-err34-c)
        const char *shiftColorStr = database.value(i, 3);
        const auto shiftColor = static_cast<ShiftData::shift_color_t>(atoi(shiftColorStr)); // NOLINT(*-err34-c)
        const char *shiftStartStr = database.value(i, 4);
        const char *shiftEndStr = database.value(i, 5);
        const auto shiftStart = Time::StringToInstant(shiftStartStr);
        const auto shiftEnd = Time::StringToInstant(shiftEndStr);
        const auto shiftRange = Time::Range(shiftStart, shiftEnd);
        const bool shiftLocked = database.value(i, 6);

        shiftAccumulator.addShift(shiftId, shiftSummary, shiftWeight, shiftColor, shiftRange, timeZone, shiftLocked);

        if (!database.isNull(i, 7)) {
            const char *shiftUserId = database.value(i, 7);
            shiftAccumulator.addShiftAssignedUserId(shiftId, shiftUserId);
        }
        if (!database.isNull(i, 8)) {
            const char *shiftAssignedSkillId = database.value(i, 8);
            shiftAccumulator.addShiftAssignedSkillId(shiftId, shiftAssignedSkillId);
        }
    }

    std::cout << "Total shift count: " << shiftAccumulator.size() << std::endl;

    std::set<std::string> actualShiftSkillIds;

    for (size_t i = 0; i < shiftAccumulator.size(); ++i) {
        const std::string shiftId = shiftAccumulator[i];
        database.execute(
            "select sk.skill_id, sk.weight from " + shiftSkillsTableName + " sk "
            "join skills s on s.id = sk.skill_id "
            "where s.organization_id = '79cf52f8-8c59-457e-9b2a-d5ec1f983776' "
            "and (s.deleted_at is null or s.deleted_at > '" + rangeStart + "') "
            "and sk.shift_id = '" + shiftId + '\''
        );

        const size_t rowCount = database.tupleCount();
        std::vector<ShiftData::Skill> shiftSkills;

        for (uint32_t j = 0; j < rowCount; ++j) {
            const auto skillId = std::string(database.value(j, 0));
            const char *skillWeightStr = database.value(j, 1);
            const auto skillWeight = static_cast<float>(atof(skillWeightStr)); // NOLINT(*-err34-c)
            actualShiftSkillIds.insert(skillId);
            shiftSkills.emplace_back(skillId, skillWeight);
        }

        shiftAccumulator.addSkills(shiftId, shiftSkills);
    }

    skillAccumulator.retainValidSkills(actualShiftSkillIds);
    userAccumulator.retainValidUsersBySkillIds(actualShiftSkillIds);

    const auto shiftTemplates = shiftAccumulator.templates();

    shiftCount = shiftTemplates.size();
    employeeCount = userAccumulator.size();
    dayCount = range.duration<std::chrono::days>().count();
    skillCount = skillAccumulator.size();

    auto *shifts = shiftAllocator.allocate(shiftCount);
    auto *employees = employeeAllocator.allocate(employeeCount);
    auto *days = dayAllocator.allocate(dayCount);
    auto *skills = skillAllocator.allocate(skillCount);

    for (axis_size_t i = 0; i < skillCount; ++i) {
        const auto &s = skillAccumulator[i];
        new(skills + i) Skill(i, s.name);
    }
    for (axis_size_t i = 0; i < shiftCount; ++i) {
        const auto &t = shiftTemplates[i];
        std::string name = t.summary;
        if (name.empty()) {
            if (t.interval.startInMinutes() > 3 * 60 && t.interval.startInMinutes() < 12 * 60) {
                if (t.interval.durationInMinutes() > 16 * 60) name = "DN";
                else name = "E";
            } else {
                if (t.interval.durationInMinutes() > 16 * 60) name = "ND";
                else name = "L";
            }
        }
        const Time::day_minutes_t restMinutesBefore = t.interval.durationInMinutes() >= 16 * 60 ? static_cast<Time::day_minutes_t>(2 * t.interval.durationInMinutes()) : t.interval.durationInMinutes();
        const Time::day_minutes_t restMinutesAfter = t.interval.durationInMinutes() >= 16 * 60 ? static_cast<Time::day_minutes_t>(2 * t.interval.durationInMinutes()) : t.interval.durationInMinutes();
        auto &s = *(new(shifts + i) Shift(i, Shift::ALL_WEEKDAYS, t.interval, name, t.slots, t.slots >> 1 > 0 ? t.slots >> 1 : 1, restMinutesBefore, restMinutesAfter));
        for (size_t j = 0; j < t.skills.size(); ++j) {
            const auto &shiftSkill = t.skills[j];
            const size_t skillIndex = skillAccumulator.getIndex(shiftSkill.m_SkillId);
            if (skillIndex == -1) continue;
            // ReSharper disable once CppTooWideScopeInitStatement
            const auto &skill = skillAccumulator[skillIndex];
            if (skill.complementary) {
                s.addRequiredAllSkill(skillIndex, shiftSkill.m_Weight);
            } else {
                s.addRequiredOneSkill(skillIndex, shiftSkill.m_Weight);
            }
        }
    }
    for (axis_size_t i = 0; i < employeeCount; ++i) {
        const auto &u = userAccumulator[i];
        auto &e = *(new(employees + i) Employee(i, u.name));

        e.setUnpaidUnavailableAvailability(u.unpaidUnavailableAvailability);
        e.setPaidUnavailableAvailability(u.paidUnavailableAvailability);
        e.setDesiredAvailability(u.desiredAvailability);

        for (size_t j = 0; j < u.skills.size(); ++j) {
            // ReSharper disable once CppUseStructuredBinding
            const auto &userSkill = u.skills[j];
            const size_t skillIndex = skillAccumulator.getIndex(userSkill.skillId);
            if (skillIndex == -1) continue;
            e.addSkill(skillIndex, {userSkill.weight, userSkill.strategy, userSkill.event});
        }
    }
    for (axis_size_t i = 0; i < dayCount; ++i) {
        new(days + i) Day(i, range.getDayRangeAt(i, timeZone));
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
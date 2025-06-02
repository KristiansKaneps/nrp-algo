#include "NrpProblemInstanceParser.h"

#include <iostream>
#include <sstream>
#include <vector>

namespace NrpProblemInstances {
    void NrpProblemInstanceParser::parse() {
        std::istringstream input(m_Data);
        std::string line;
        std::string currentSection;

        while (std::getline(input, line)) {
            // Trim leading and trailing whitespace
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line.empty() || line[0] == '#') continue;

            if (line.rfind("SECTION_", 0) == 0) {
                currentSection = line;
                std::cout << "\n[Entering Section: " << currentSection << "]\n";
                continue;
            }

            std::istringstream linestream(line);
            std::string field;
            std::vector<std::string> fields;

            while (std::getline(linestream, field, ',')) {
                // Remove whitespace around each field
                field.erase(0, field.find_first_not_of(" \t\r\n"));
                field.erase(field.find_last_not_of(" \t\r\n") + 1);
                fields.push_back(field);
            }

            // Section-based parsing
            if (currentSection == "SECTION_HORIZON") {
                int horizonDays = std::stoi(fields[0]);
                const auto local = std::chrono::local_time<std::chrono::milliseconds>{m_Range.start().time_since_epoch()};
                const auto zonedStart = std::chrono::zoned_time(m_TimeZone, local);
                m_Range = Time::Range(zonedStart.get_sys_time(), zonedStart.get_sys_time() + std::chrono::days(horizonDays));
                std::cout << "Horizon: " << horizonDays << " days; range: " << m_Range << std::endl;
            } else if (currentSection == "SECTION_SHIFTS") {
                const size_t shiftIndex = m_ShiftCounter++;
                const size_t skillIndex = m_SkillCounter++; // Each shift has one skill due to data input format.
                const std::string& shiftID = fields[0];
                const int32_t duration = std::stoi(fields[1]);
                std::cout << "Shift: " << shiftID << ", duration: " << duration;
                if (fields.size() > 2) {
                    const std::string& blocked = fields[2];
                    std::cout << ", blocked: " << blocked;
                    m_ShiftIndexToBlockedShiftNamesMap.insert({shiftIndex, blocked});
                }
                std::cout << "\n";
                m_ShiftNameToIndexMap.insert({shiftID, shiftIndex});
                m_SkillNameToIndexMap.insert({shiftID, skillIndex});
                Domain::Shift shift(
                    shiftIndex,
                    Domain::Shift::ALL_WEEKDAYS,
                    Time::DailyInterval("00:00", static_cast<Time::day_minutes_t>(duration)),
                    shiftID,
                    1, 1,
                    0, 0,
                    0
                );
                Domain::Skill skill(skillIndex, shiftID);
                shift.addRequiredOneSkill(skillIndex, 1.0f);
                m_Shifts.emplace_back(shift);
                m_Skills.emplace_back(skill);
            } else if (currentSection == "SECTION_STAFF") {
                size_t employeeIndex = m_EmployeeCounter++;
                const std::string& id = fields[0];
                const std::string& skillEntriesDelimited = fields[1];
                const auto maxTotalMinutes = std::stoi(fields[2]);
                const auto minTotalMinutes = std::stoi(fields[3]);
                const auto maxOvertimeMinutes = maxTotalMinutes - minTotalMinutes;
                const auto maxConsecutiveShiftCount = std::stoi(fields[4]);
                const auto minConsecutiveShiftCount = std::stoi(fields[5]);
                const auto minConsecutiveDaysOffCount = std::stoi(fields[6]);
                const auto maxWorkingWeekendCount = std::stoi(fields[7]);
                std::cout << "Staff: " << id << " | ";
                for (size_t i = 1; i < fields.size(); ++i) {
                    std::cout << fields[i] << " ";
                }
                std::cout << "\n";
                Domain::Employee employee(employeeIndex, id);
                employee.setGeneralConstraints(Domain::Employee::GeneralConstraints{
                    static_cast<uint8_t>(minConsecutiveShiftCount),
                    static_cast<uint8_t>(maxConsecutiveShiftCount),
                    static_cast<uint8_t>(minConsecutiveDaysOffCount),
                    static_cast<uint8_t>(maxWorkingWeekendCount)
                });
                for (const std::vector<std::string> skillEntries = split(skillEntriesDelimited, '|'); const auto& skillEntry : skillEntries) {
                    const std::vector<std::string> elements = split(skillEntry, '=');
                    const std::string& skillID = elements[0];
                    const std::string& maxAssignedShiftCount = elements[1];
                    const auto skillIndex = m_SkillNameToIndexMap[skillID];
                    const auto shiftCount = std::stoi(maxAssignedShiftCount);
                    Domain::EmployeeSkill employeeSkill{
                        1.0f,
                        Domain::Workload::DYNAMIC,
                        Domain::Workload::ChangeEvent{
                            static_cast<float>(minTotalMinutes) / 60.0f,
                            0.0f,
                            static_cast<float>(maxOvertimeMinutes) / 60.0f,
                            shiftCount,
                        }
                    };
                    employee.addSkill(skillIndex, employeeSkill);
                }
                employee.setTotalChangeEvent(Domain::Workload::TotalChangeEvent{
                    false,
                    static_cast<float>(minTotalMinutes) / 60.0f,
                    static_cast<float>(maxOvertimeMinutes) / 60.0f,
                    -1,
                });
                m_EmployeeNameToIndexMap.insert({id, employeeIndex});
                m_Employees.emplace_back(employee);
            } else if (currentSection == "SECTION_DAYS_OFF") {
                const size_t empIndex = m_EmployeeNameToIndexMap[fields[0]];
                auto& emp = m_Employees[empIndex];
                std::cout << "Days off for " << fields[0] << ": ";
                Time::RangeCollection unavailabilityRangeCollection(fields.size() - 1);
                for (size_t i = 1; i < fields.size(); ++i) {
                    std::cout << fields[i] << " ";
                    const size_t dayIndex = std::stoi(fields[i]);
                    const auto& unavailabilityRange = m_Range.getDayRangeAt(dayIndex, m_TimeZone);
                    unavailabilityRangeCollection.add(unavailabilityRange);
                }
                std::cout << "\n";
                emp.addUnpaidUnavailableAvailability(unavailabilityRangeCollection);
            } else if (currentSection == "SECTION_SHIFT_ON_REQUESTS" || currentSection ==
                "SECTION_SHIFT_OFF_REQUESTS") {
                std::string type = (currentSection == "SECTION_SHIFT_ON_REQUESTS") ? "On" : "Off";
                const size_t empIndex = m_EmployeeNameToIndexMap[fields[0]];
                const size_t dayIndex = std::stoi(fields[1]);
                const size_t shiftIndex = m_ShiftNameToIndexMap[fields[2]];
                int32_t weight = std::stoi(fields[3]);
                if (weight > m_MaxRawAvailabilitySpecificRequestWeight) m_MaxRawAvailabilitySpecificRequestWeight = weight;
                m_RawAvailabilitySpecificRequests.emplace_back(RawAvailabilitySpecificRequest{
                    empIndex,
                    dayIndex,
                    shiftIndex,
                    weight,
                    type
                });
                std::cout << "Shift " << type << " request: " << fields[0] << " day " << dayIndex << " shift " << fields[2] <<
                    " weight " << weight << "\n";
            } else if (currentSection == "SECTION_COVER") {
                int day = std::stoi(fields[0]);
                const std::string& shiftID = fields[1];
                int required = std::stoi(fields[2]);
                int underWeight = std::stoi(fields[3]);
                int overWeight = std::stoi(fields[4]);
                const auto shiftIndex = m_ShiftNameToIndexMap[shiftID];
                auto &shift = m_Shifts[shiftIndex];
                shift.setSlotCountAtDay(static_cast<Domain::axis_size_t>(day), required);
                std::cout << "Cover: Day " << day << ", Shift " << shiftID
                    << ", Need " << required << ", UnderWt " << underWeight
                    << ", OverWt " << overWeight << "\n";
            }
        }

        for (size_t i = 0; i < m_Shifts.size(); ++i) {
            if (!m_ShiftIndexToBlockedShiftNamesMap.contains(i)) continue;
            auto& shift = m_Shifts[i];
            for (const std::vector<std::string> blockedShiftNames = split(m_ShiftIndexToBlockedShiftNamesMap[i], '|'); const auto& blockedShiftName : blockedShiftNames) {
                shift.addBlockedNextDayShiftIndex(m_ShiftNameToIndexMap[blockedShiftName]);
            }
        }

        for (const auto& request : m_RawAvailabilitySpecificRequests) {
            // const float weight = m_MaxRawAvailabilitySpecificRequestWeight > 0 ? static_cast<float>(request.weight) / static_cast<float>(m_MaxRawAvailabilitySpecificRequestWeight) : 1.0f;
            const auto weight = static_cast<int8_t>(request.weight);
            auto& emp = m_Employees[request.empIndex];
            if (request.type == "On") {
                emp.addDesiredAvailability(Domain::Availability::DesiredAvailability::SpecificRequest{request.shiftIndex, request.dayIndex, weight});
            } else if (request.type == "Off") {
                emp.addUnpaidUnavailableAvailability(Domain::Availability::UnpaidUnavailableAvailability::SpecificRequest{request.shiftIndex, request.dayIndex, weight});
            }
        }
    }
}

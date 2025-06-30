#ifndef NRPPROBLEMINSTANCEPARSER_H
#define NRPPROBLEMINSTANCEPARSER_H

#include "embedded_resources.h"
#include <string>
#include <vector>
#include <unordered_map>

#include "Domain/Entities/Employee.h"
#include "Domain/Entities/Shift.h"
#include "Domain/Entities/Skill.h"

#include "Time/Range.h"

namespace NrpProblemInstances {
    class NrpProblemInstanceParser {
    public:
        enum Type : uint8_t {
            TXT = 0,
            XML,
        };

        explicit NrpProblemInstanceParser(const std::string& resourceName, const std::chrono::time_zone *const timeZone, const Type type = TXT) : m_Type(type), m_TimeZone(timeZone) {
            m_Data = find_embedded_file(("NrpProblemInstances/" + resourceName).c_str(), &m_DataSize);
        }

        explicit NrpProblemInstanceParser(const std::string& resourceName, const Type type = TXT) : NrpProblemInstanceParser(resourceName, std::chrono::current_zone(), type) { }

        ~NrpProblemInstanceParser() = default;

        void parse() {
            if (m_Type == TXT) parseTxt();
            else if (m_Type == XML) parseXml();
        }

        [[nodiscard]] const std::chrono::time_zone *timeZone() const { return m_TimeZone; }
        [[nodiscard]] Time::Range range() const { return m_Range; }

        [[nodiscard]] size_t shiftCount() const { return m_Shifts.size(); }
        [[nodiscard]] std::vector<Domain::Shift> shifts() const { return m_Shifts; }

        [[nodiscard]] size_t skillCount() const { return m_Skills.size(); }
        [[nodiscard]] std::vector<Domain::Skill> skills() const { return m_Skills; }

        [[nodiscard]] size_t employeeCount() const { return m_Employees.size(); }
        [[nodiscard]] std::vector<Domain::Employee> employees() const { return m_Employees; }

    protected:
        const Type m_Type;
        const char *m_Data {};
        size_t m_DataSize {};

        const std::chrono::time_zone *const m_TimeZone;

        Time::Range m_Range {
            Time::StringToInstant("2025-01-01T00:00:00.000Z"), Time::StringToInstant("2025-01-01T00:00:00.000Z")
        };

        size_t m_ShiftCounter{};
        std::unordered_map<size_t, std::string> m_ShiftIndexToBlockedShiftNamesMap{};
        std::unordered_map<std::string, size_t> m_ShiftNameToIndexMap{};
        std::vector<Domain::Shift> m_Shifts{};

        size_t m_SkillCounter{};
        std::unordered_map<std::string, size_t> m_SkillNameToIndexMap{};
        std::vector<Domain::Skill> m_Skills{};

        size_t m_EmployeeCounter{};
        std::unordered_map<std::string, size_t> m_EmployeeNameToIndexMap{};
        std::vector<Domain::Employee> m_Employees{};

        void parseTxt();
        void parseXml();

    private:
        struct RawAvailabilitySpecificRequest {
            size_t empIndex;
            size_t dayIndex;
            size_t shiftIndex;
            int32_t weight;
            std::string type;
        };

        int32_t m_MaxRawAvailabilitySpecificRequestWeight{};
        std::vector<RawAvailabilitySpecificRequest> m_RawAvailabilitySpecificRequests{};

        static std::vector<std::string> split(const std::string& str, const char delimiter) {
            std::vector<std::string> result;
            std::stringstream ss(str);
            std::string item;

            while (std::getline(ss, item, delimiter)) {
                result.push_back(item);
            }

            return result;
        }
    };
}

#endif //NRPPROBLEMINSTANCEPARSER_H

#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <string>
#include <unordered_map>

#include "Time/RangeCollection.h"

#include "State/Axes.h"

namespace Domain {
    using axis_size_t = Axes::axis_size_t;

    namespace Workload {
        enum Strategy : uint8_t {
            NONE = 0,
            STATIC,
            DYNAMIC,
        };

        struct ChangeEvent {
            float dynamicLoadHours{};
            float staticLoad{};
            float maxOvertimeHours{};
            int maxShiftCount = -1;
        };

        struct TotalChangeEvent {
            bool anyDuration = true;
            float maxLoadHours{};
            float maxOvertimeHours{};
            int maxShiftCount = -1;
        };
    }

    struct EmployeeSkill {
        float weight{};
        Workload::Strategy strategy{};
        Workload::ChangeEvent event{};
    };

    namespace Availability {
        enum Type : uint8_t {
            AVAILABLE = 0,
            UNAVAILABLE,
            DESIRED,
        };

        enum UnavailabilitySubtype : uint8_t {
            NONE = 0,
            VACATION,
            SICK_PAGE_A,
            SICK_PAGE_B,
            PATERNITY_LEAVE,
            ADDITIONAL_LEAVE,
            STUDY_LEAVE,
            DECREE_LEAVE,
            BUSINESS_TRIP,
        };

        class Availability {
        public:
            struct SpecificRequest;

            Time::RangeCollection m_RangeCollection;
            Type m_Type;
            bool m_PaidUnavailability;
            std::vector<SpecificRequest> m_SpecificRequests{};

            struct SpecificRequest {
                size_t shiftIndex;
                size_t dayIndex;
                int8_t weight;
            };

            Availability(const Time::RangeCollection& rangeCollection, const Type type, const bool paidUnavailability) noexcept :
                m_RangeCollection(rangeCollection),
                m_Type(type),
                m_PaidUnavailability(paidUnavailability) { }

            void addSpecificRequest(const size_t shiftIndex, const size_t dayIndex, const int8_t weight) noexcept {
                m_SpecificRequests.emplace_back(SpecificRequest{shiftIndex, dayIndex, weight});
            }
        };

        class DesiredAvailability final : public Availability {
        public:
            DesiredAvailability(const Time::RangeCollection& rangeCollection) noexcept : Availability(rangeCollection, Type::DESIRED, false) { }
            DesiredAvailability() noexcept : DesiredAvailability(Time::RangeCollection()) { }
        };

        class PaidUnavailableAvailability final : public Availability {
        public:
            PaidUnavailableAvailability(const Time::RangeCollection& rangeCollection) noexcept : Availability(rangeCollection, Type::UNAVAILABLE, true) { }
            PaidUnavailableAvailability() noexcept : PaidUnavailableAvailability(Time::RangeCollection()) { }
        };

        class UnpaidUnavailableAvailability final : public Availability {
        public:
            UnpaidUnavailableAvailability(const Time::RangeCollection& rangeCollection) noexcept : Availability(rangeCollection, Type::UNAVAILABLE, false) { }
            UnpaidUnavailableAvailability() noexcept : UnpaidUnavailableAvailability(Time::RangeCollection()) { }
        };
    }

    class Employee : public Axes::AxisEntity {
    public:
        struct GeneralConstraints;

        explicit Employee(const axis_size_t index) noexcept : m_Index(index), m_Name(std::to_string(index + 1)) { }
        Employee(const axis_size_t index, const std::string& name) noexcept : m_Index(index), m_Name(name) { }

        ~Employee() noexcept override = default;

        [[nodiscard]] axis_size_t index() const noexcept { return m_Index; }
        [[nodiscard]] std::string name() const noexcept { return m_Name; }
        [[nodiscard]] Workload::TotalChangeEvent totalChangeEvent() const noexcept { return m_TotalChangeEvent; }
        [[nodiscard]] const std::unordered_map<axis_size_t, EmployeeSkill>& skills() const noexcept { return m_Skills; }
        [[nodiscard]] GeneralConstraints generalConstraints() const noexcept { return m_GeneralConstraints; }

        void setTotalChangeEvent(const Workload::TotalChangeEvent& event) noexcept { m_TotalChangeEvent = event; }

        [[nodiscard]] const EmployeeSkill* skill(const axis_size_t skillIndex) const noexcept {
            const auto skill = m_Skills.find(skillIndex);
            return skill == m_Skills.end() ? nullptr : &skill->second;
        }

        [[nodiscard]] bool hasSkill(const axis_size_t skillIndex) const noexcept { return m_Skills.contains(skillIndex); }

        [[nodiscard]] float getSkillWeight(const axis_size_t skillIndex) const noexcept {
            const auto weight = m_Skills.find(skillIndex);
            if (weight == m_Skills.end()) return 0.0f;
            return weight->second.weight;
        }

        void removeSkill(const axis_size_t skillIndex) noexcept { m_Skills.erase(skillIndex); }

        void setSkillWeight(const axis_size_t skillIndex, const float weight) noexcept { m_Skills[skillIndex].weight = weight; }

        void addSkill(const axis_size_t skillIndex, const EmployeeSkill& skill) noexcept { m_Skills[skillIndex] = skill; }

        void setGeneralConstraints(const GeneralConstraints& generalConstraints) noexcept { m_GeneralConstraints = generalConstraints; }

        void setUnpaidUnavailableAvailability(const Availability::UnpaidUnavailableAvailability &availability) noexcept {
            m_UnpaidUnavailableAvailability = availability;
        }
        void addUnpaidUnavailableAvailability(const Time::RangeCollection &availabilityRangeCollection) noexcept {
            m_UnpaidUnavailableAvailability.m_RangeCollection.addAll(availabilityRangeCollection);
        }
        void addUnpaidUnavailableAvailability(const Availability::UnpaidUnavailableAvailability::SpecificRequest &specificRequest) noexcept {
            m_UnpaidUnavailableAvailability.m_SpecificRequests.emplace_back(specificRequest);
        }

        void setPaidUnavailableAvailability(const Availability::PaidUnavailableAvailability &availability) noexcept {
            m_PaidUnavailableAvailability = availability;
        }
        void addPaidUnavailableAvailability(const Time::RangeCollection &availabilityRangeCollection) noexcept {
            m_PaidUnavailableAvailability.m_RangeCollection.addAll(availabilityRangeCollection);
        }
        void addPaidUnavailableAvailability(const Availability::PaidUnavailableAvailability::SpecificRequest &specificRequest) noexcept {
            m_PaidUnavailableAvailability.m_SpecificRequests.emplace_back(specificRequest);
        }

        void setDesiredAvailability(const Availability::DesiredAvailability &availability) noexcept {
            m_DesiredAvailability = availability;
        }
        void addDesiredAvailability(const Time::RangeCollection &availabilityRangeCollection) noexcept {
            m_DesiredAvailability.m_RangeCollection.addAll(availabilityRangeCollection);
        }
        void addDesiredAvailability(const Availability::DesiredAvailability::SpecificRequest &specificRequest) noexcept {
            m_DesiredAvailability.m_SpecificRequests.emplace_back(specificRequest);
        }

        [[nodiscard]] const Availability::UnpaidUnavailableAvailability& unpaidUnavailableAvailability() const noexcept { return m_UnpaidUnavailableAvailability; }
        [[nodiscard]] const Availability::PaidUnavailableAvailability& paidUnavailableAvailability() const noexcept { return m_PaidUnavailableAvailability; }
        [[nodiscard]] const Availability::DesiredAvailability& desiredAvailability() const noexcept { return m_DesiredAvailability; }

        struct GeneralConstraints {
            uint8_t minConsecutiveShiftCount = 0;
            uint8_t maxConsecutiveShiftCount = 0;
            uint8_t minConsecutiveDaysOffCount = 0;
            int16_t maxWorkingWeekendCount = -1;
        };

    private:
        const axis_size_t m_Index;
        const std::string m_Name;
        GeneralConstraints m_GeneralConstraints {};
        Workload::TotalChangeEvent m_TotalChangeEvent {};
        std::unordered_map<axis_size_t, EmployeeSkill> m_Skills {};

        Availability::UnpaidUnavailableAvailability m_UnpaidUnavailableAvailability;
        Availability::PaidUnavailableAvailability m_PaidUnavailableAvailability;
        Availability::DesiredAvailability m_DesiredAvailability;
    };
}

#endif //EMPLOYEE_H

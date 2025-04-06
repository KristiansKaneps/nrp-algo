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
            float dynamicLoadHours;
            float staticLoad;
            float maxOvertimeHours;
        };
    }

    struct EmployeeSkill {
        float weight;
        Workload::Strategy strategy;
        Workload::ChangeEvent event;
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
            Time::RangeCollection m_RangeCollection;
            Type m_Type;
            bool m_PaidUnavailability;

            Availability(const Time::RangeCollection& rangeCollection, const Type type, const bool paidUnavailability) :
                m_RangeCollection(rangeCollection),
                m_Type(type),
                m_PaidUnavailability(paidUnavailability) { }
        };

        class DesiredAvailability final : public Availability {
        public:
            DesiredAvailability(const Time::RangeCollection& rangeCollection) : Availability(rangeCollection, Type::DESIRED, false) { }
            DesiredAvailability() : DesiredAvailability(Time::RangeCollection()) { }
        };

        class PaidUnavailableAvailability final : public Availability {
        public:
            PaidUnavailableAvailability(const Time::RangeCollection& rangeCollection) : Availability(rangeCollection, Type::UNAVAILABLE, true) { }
            PaidUnavailableAvailability() : PaidUnavailableAvailability(Time::RangeCollection()) { }
        };

        class UnpaidUnavailableAvailability final : public Availability {
        public:
            UnpaidUnavailableAvailability(const Time::RangeCollection& rangeCollection) : Availability(rangeCollection, Type::UNAVAILABLE, false) { }
            UnpaidUnavailableAvailability() : UnpaidUnavailableAvailability(Time::RangeCollection()) { }
        };
    }

    class Employee : public Axes::AxisEntity {
    public:
        explicit Employee(const axis_size_t index) : m_Index(index), m_Name(std::to_string(index + 1)) { }
        Employee(const axis_size_t index, const std::string& name) : m_Index(index), m_Name(name) { }

        ~Employee() override = default;

        [[nodiscard]] axis_size_t index() const { return m_Index; }
        [[nodiscard]] std::string name() const { return m_Name; }
        [[nodiscard]] const std::unordered_map<axis_size_t, EmployeeSkill>& skills() const { return m_Skills; }

        [[nodiscard]] const EmployeeSkill* skill(const axis_size_t skillIndex) const {
            const auto skill = m_Skills.find(skillIndex);
            return skill == m_Skills.end() ? nullptr : &skill->second;
        }

        [[nodiscard]] bool hasSkill(const axis_size_t skillIndex) const { return m_Skills.contains(skillIndex); }

        [[nodiscard]] float getSkillWeight(const axis_size_t skillIndex) const {
            const auto weight = m_Skills.find(skillIndex);
            if (weight == m_Skills.end()) return 0.0f;
            return weight->second.weight;
        }

        void removeSkill(const axis_size_t skillIndex) { m_Skills.erase(skillIndex); }

        void setSkillWeight(const axis_size_t skillIndex, const float weight) { m_Skills[skillIndex].weight = weight; }

        void addSkill(const axis_size_t skillIndex, const EmployeeSkill& skill) { m_Skills[skillIndex] = skill; }

        void setUnpaidUnavailableAvailability(const Availability::UnpaidUnavailableAvailability &availability) {
            m_UnpaidUnavailableAvailability = availability;
        }
        void setPaidUnavailableAvailability(const Availability::PaidUnavailableAvailability &availability) {
            m_PaidUnavailableAvailability = availability;
        }
        void setDesiredAvailability(const Availability::DesiredAvailability &availability) {
            m_DesiredAvailability = availability;
        }

        [[nodiscard]] const Availability::UnpaidUnavailableAvailability& unpaidUnavailableAvailability() const { return m_UnpaidUnavailableAvailability; }
        [[nodiscard]] const Availability::PaidUnavailableAvailability& paidUnavailableAvailability() const { return m_PaidUnavailableAvailability; }
        [[nodiscard]] const Availability::DesiredAvailability& desiredAvailability() const { return m_DesiredAvailability; }

    private:
        const axis_size_t m_Index;
        const std::string m_Name;
        std::unordered_map<axis_size_t, EmployeeSkill> m_Skills {};

        Availability::UnpaidUnavailableAvailability m_UnpaidUnavailableAvailability;
        Availability::PaidUnavailableAvailability m_PaidUnavailableAvailability;
        Availability::DesiredAvailability m_DesiredAvailability;
    };
}

#endif //EMPLOYEE_H

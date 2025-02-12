#ifndef USERDATAACCUMULATOR_H
#define USERDATAACCUMULATOR_H

#include <string>
#include <set>
#include <unordered_map>
#include <utility>

#include "Time/RangeCollection.h"

namespace UserData {
    enum AvailabilityType {
        AVAILABLE = 0,
        UNAVAILABLE,
        DESIRED,
    };

    enum UnavailabilitySubtype {
        NONE = 0,
        VACATION,
        SICK_PAGE_A,
        SICK_PAGE_B,
        PATERNITY_LEAVE,
        ADDITIONAL_LEAVE,
        STUDY_LEAVE,
        DECREE_LEAVE,
    };

    class Availability {
    public:
        Time::RangeCollection m_RangeCollection;
        AvailabilityType m_Type;
        bool m_PaidUnavailability;

        Availability(const Time::RangeCollection& rangeCollection, const AvailabilityType type,
                     const bool paidUnavailability) :
            m_RangeCollection(rangeCollection),
            m_Type(type),
            m_PaidUnavailability(paidUnavailability) { }
    };

    class DesiredAvailability final : public Availability {
    public:
        DesiredAvailability() : Availability(Time::RangeCollection(), AvailabilityType::DESIRED, false) { }
    };

    class PaidUnavailableAvailability final : public Availability {
    public:
        PaidUnavailableAvailability() : Availability(Time::RangeCollection(), AvailabilityType::UNAVAILABLE, true) { }
    };

    class UnpaidUnavailableAvailability final : public Availability {
    public:
        UnpaidUnavailableAvailability() : Availability(Time::RangeCollection(), AvailabilityType::UNAVAILABLE,
                                                       false) { }
    };

    class Skill {
    public:
        std::string m_SkillId;
        float m_Weight;

        Skill(std::string skillId, const float weight) : m_SkillId(std::move(skillId)), m_Weight(weight) { }
    };

    class Accumulator {
    public:
        Accumulator() = default;
        ~Accumulator() = default;

        [[nodiscard]] size_t size() const {
            return m_Ids.size();
        }

        std::string operator[](const size_t index) const {
            auto iterator = m_Ids.begin();
            std::advance(iterator, index);
            return *iterator;
        }

        std::string name(const size_t index) const {
            return m_NameMap.at(operator[](index));
        }

        void addUserFullName(const std::string& id, const std::string& email, const std::string& name, const std::string& surname) {
            m_Ids.insert(id);
            m_NameMap.insert({id, name});
            m_SurnameMap.insert({id, surname});
            m_EmailMap.insert({id, email});
        }

        void addAvailability(const std::string& id, const Time::Range& range, const AvailabilityType type,
                             const UnavailabilitySubtype subtype) {
            ensureUser(id);
            switch (type) {
                case AvailabilityType::UNAVAILABLE:
                    if (subtype > UnavailabilitySubtype::NONE) {
                        m_AvailabilityPaidUnavailableMap[id].m_RangeCollection.add(range);
                    } else { m_AvailabilityUnpaidUnavailableMap[id].m_RangeCollection.add(range); }
                    break;
                case AvailabilityType::DESIRED:
                    m_AvailabilityDesiredMap[id].m_RangeCollection.add(range);
                default:
                    return;
            }
        }

        void addSkills(const std::string &id, const std::vector<Skill>& skills) {
            m_SkillMap.insert({id, skills});
        }

    private:
        std::set<std::string> m_Ids;

        std::unordered_map<std::string, std::string> m_NameMap;
        std::unordered_map<std::string, std::string> m_SurnameMap;
        std::unordered_map<std::string, std::string> m_EmailMap;
        std::unordered_map<std::string, UnpaidUnavailableAvailability> m_AvailabilityUnpaidUnavailableMap;
        std::unordered_map<std::string, PaidUnavailableAvailability> m_AvailabilityPaidUnavailableMap;
        std::unordered_map<std::string, DesiredAvailability> m_AvailabilityDesiredMap;

        std::unordered_map<std::string, std::vector<Skill>> m_SkillMap;

        void ensureUser(const std::string& id) {
            if (!m_AvailabilityUnpaidUnavailableMap.contains(id))
                m_AvailabilityUnpaidUnavailableMap.insert({id, UnpaidUnavailableAvailability()});
            if (!m_AvailabilityPaidUnavailableMap.contains(id))
                m_AvailabilityPaidUnavailableMap.insert({id, PaidUnavailableAvailability()});
            if (!m_AvailabilityDesiredMap.contains(id))
                m_AvailabilityDesiredMap.insert({id, DesiredAvailability()});
        }
    };
}

#endif //USERDATAACCUMULATOR_H

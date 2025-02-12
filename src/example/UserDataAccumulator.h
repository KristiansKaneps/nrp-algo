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

    struct Skill {
        std::string skillId;
        float weight;
    };

    struct Employee {
        std::string userId;
        std::string name;
        std::string surname;
        std::string email;

        std::vector<Skill> skills;
        UnpaidUnavailableAvailability unpaidUnavailableAvailability;
        PaidUnavailableAvailability paidUnavailableAvailability;
        DesiredAvailability desiredAvailability;
    };

    class Accumulator {
    public:
        Accumulator() = default;
        ~Accumulator() = default;

        [[nodiscard]] size_t size() const {
            return m_Ids.size();
        }

        Employee operator[](const size_t index) const {
            auto iterator = m_Ids.begin();
            std::advance(iterator, index);
            const auto &id = *iterator;
            const auto &skills = m_SkillMap.contains(id) ? m_SkillMap.at(id) : std::vector<Skill>(0);
            const auto &unpaidUnavailableAvailability = m_AvailabilityUnpaidUnavailableMap.contains(id) ? m_AvailabilityUnpaidUnavailableMap.at(id) : UnpaidUnavailableAvailability();
            const auto &paidUnavailableAvailability = m_AvailabilityPaidUnavailableMap.contains(id) ? m_AvailabilityPaidUnavailableMap.at(id) : PaidUnavailableAvailability();
            const auto &desiredAvailability = m_AvailabilityDesiredMap.contains(id) ? m_AvailabilityDesiredMap.at(id) : DesiredAvailability();
            return {
                id,
                m_NameMap.at(id),
                m_SurnameMap.at(id),
                m_EmailMap.at(id),
                skills,
                unpaidUnavailableAvailability,
                paidUnavailableAvailability,
                desiredAvailability,
            };
        }

        [[nodiscard]] std::set<std::string> ids() const {
            return m_Ids;
        }

        void retainValidUsersBySkillIds(const std::set<std::string> &skillIds) {
            for(auto it = m_Ids.begin(); it != m_Ids.end();) {
                const auto &userId = *it;
                const auto userSkillEntries = m_SkillMap.find(userId);
                if (userSkillEntries == m_SkillMap.end()) {
                    it = m_Ids.erase(it);
                    continue;
                }
                // ReSharper disable once CppTooWideScopeInitStatement
                const auto &userSkills = userSkillEntries->second;

                bool intersects = false;

                // ReSharper disable once CppUseStructuredBinding
                for (const auto &userSkill : userSkills) {
                    if (skillIds.contains(userSkill.skillId)) {
                        intersects = true;
                        break;
                    }
                }

                if (!intersects) {
                    it = m_Ids.erase(it);
                    continue;
                }

                ++it;
            }
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

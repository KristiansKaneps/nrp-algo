#ifndef USERDATAACCUMULATOR_H
#define USERDATAACCUMULATOR_H

#include <string>
#include <unordered_map>

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

    class Accumulator {
    public:
        Accumulator() = default;
        ~Accumulator() = default;

        void addUserFullName(const std::string& email, const std::string& name, const std::string& surname) {
            m_NameMap.insert({email, name});
            m_SurnameMap.insert({email, surname});
        }

        void addAvailability(const std::string& email, const Time::Range& range, const AvailabilityType type,
                             const UnavailabilitySubtype subtype) {
            ensureUser(email);
            switch (type) {
                case AvailabilityType::UNAVAILABLE:
                    if (subtype > UnavailabilitySubtype::NONE) {
                        m_AvailabilityPaidUnavailableMap[email].m_RangeCollection.add(range);
                    } else { m_AvailabilityUnpaidUnavailableMap[email].m_RangeCollection.add(range); }
                    break;
                case AvailabilityType::DESIRED:
                    m_AvailabilityDesiredMap[email].m_RangeCollection.add(range);
                default:
                    return;
            }
        }

    private:
        std::unordered_map<std::string, std::string> m_NameMap;
        std::unordered_map<std::string, std::string> m_SurnameMap;
        std::unordered_map<std::string, UnpaidUnavailableAvailability> m_AvailabilityUnpaidUnavailableMap;
        std::unordered_map<std::string, PaidUnavailableAvailability> m_AvailabilityPaidUnavailableMap;
        std::unordered_map<std::string, DesiredAvailability> m_AvailabilityDesiredMap;

        void ensureUser(const std::string& email) {
            if (!m_AvailabilityUnpaidUnavailableMap.contains(email))
                m_AvailabilityUnpaidUnavailableMap.insert({email, UnpaidUnavailableAvailability()});
            if (!m_AvailabilityPaidUnavailableMap.contains(email))
                m_AvailabilityPaidUnavailableMap.insert({email, PaidUnavailableAvailability()});
            if (!m_AvailabilityDesiredMap.contains(email))
                m_AvailabilityDesiredMap.insert({email, DesiredAvailability()});
        }
    };
}

#endif //USERDATAACCUMULATOR_H

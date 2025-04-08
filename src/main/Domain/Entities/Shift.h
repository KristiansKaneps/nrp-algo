#ifndef SHIFT_H
#define SHIFT_H

#include <string>
#include <unordered_map>
#include <utility>

#include "Time/DailyInterval.h"

#include "State/Axes.h"

namespace Domain {
    using axis_size_t = Axes::axis_size_t;

    class Shift : public Axes::AxisEntity {
    public:
        static constexpr uint8_t ALL_WEEKDAYS = 0b11111111;
        static constexpr uint8_t ONLY_WORKDAYS = 0b00011111;
        static constexpr uint8_t ONLY_WEEKENDS = 0b01100000;
        static constexpr uint8_t ALL_WEEKDAYS_EXCEPT_HOLIDAYS = 0b01111111; // TODO: Implement holidays as 8th bit
        static constexpr uint8_t ONLY_WEEKENDS_AND_HOLIDAYS = 0b11100000; // TODO: Implement holidays as 8th bit
        static constexpr uint8_t ONLY_HOLIDAYS = 0b10000000; // TODO: Implement holidays as 8th bit

        Shift(const axis_size_t index, const uint8_t weekdayBitMask, const Time::DailyInterval& interval,
              const std::string& name, const uint8_t slotCount, const uint8_t requiredSlotCount,
              const Time::day_minutes_t restMinutesBefore, const Time::day_minutes_t restMinutesAfter) : m_Index(index),
            m_WeekdayBitMask(weekdayBitMask),
            m_Interval(interval),
            m_Name(name),
            m_SlotCount(slotCount),
            m_RequiredSlotCount(requiredSlotCount),
            m_RestMinutesBefore(restMinutesBefore),
            m_RestMinutesAfter(restMinutesAfter) { }

        Shift(const axis_size_t index, const uint8_t weekdayBitMask, const Time::DailyInterval& interval,
              const std::string& name, const uint8_t slotCount, const Time::day_minutes_t restMinutesBefore,
              const Time::day_minutes_t restMinutesAfter) : Shift(index, weekdayBitMask, interval, name,
                                                                  slotCount, slotCount, restMinutesBefore,
                                                                  restMinutesAfter) { }

        Shift(const axis_size_t index, const uint8_t weekdayBitMask, const Time::DailyInterval& interval,
              const std::string& name, const uint8_t slotCount,
              const Time::day_minutes_t restMinutesBeforeAndAfter) : Shift(index, weekdayBitMask, interval, name,
                                                                           slotCount, slotCount,
                                                                           restMinutesBeforeAndAfter,
                                                                           restMinutesBeforeAndAfter) { }

        Shift(const axis_size_t index, const uint8_t weekdayBitMask, const Time::DailyInterval& interval,
              const std::string& name, const uint8_t slotCount) : Shift(index, weekdayBitMask, interval, name,
                                                                        slotCount, slotCount,
                                                                        interval.durationInMinutes(),
                                                                        interval.durationInMinutes()) { }

        Shift(const axis_size_t index, const uint8_t weekdayBitMask, const Time::DailyInterval& interval,
              const std::string& name) : Shift(index, weekdayBitMask, interval, name, 1, interval.durationInMinutes(),
                                               interval.durationInMinutes()) { }

        [[nodiscard]] axis_size_t index() const { return m_Index; }
        [[nodiscard]] uint8_t weekdayBitMask() const { return m_WeekdayBitMask; }
        [[nodiscard]] const Time::DailyInterval& interval() const { return m_Interval; }
        [[nodiscard]] const std::string& name() const { return m_Name; }
        [[nodiscard]] uint8_t slotCount() const { return m_SlotCount; }
        [[nodiscard]] uint8_t requiredSlotCount() const { return m_RequiredSlotCount; }
        [[nodiscard]] Time::day_minutes_t restMinutesBefore() const { return m_RestMinutesBefore; }
        [[nodiscard]] Time::day_minutes_t restMinutesAfter() const { return m_RestMinutesAfter; }

        [[nodiscard]] const std::unordered_map<axis_size_t, float>& requiredAllSkills() const {
            return m_RequiredAllSkills;
        }

        [[nodiscard]] const std::unordered_map<axis_size_t, float>& requiredOneSkills() const {
            return m_RequiredOneSkills;
        }

        [[nodiscard]] bool requiresSkill() const {
            return !m_RequiredAllSkills.empty() || !m_RequiredOneSkills.empty();
        }

        [[nodiscard]] bool requiresSkill(const axis_size_t skillIndex) const {
            return m_RequiredAllSkills.contains(skillIndex) || (m_RequiredOneSkills.size() == 1 && m_RequiredOneSkills.
                contains(skillIndex));
        }

        void removeRequiredAllSkill(const axis_size_t skillIndex) { m_RequiredAllSkills.erase(skillIndex); }
        void removeRequiredOneSkill(const axis_size_t skillIndex) { m_RequiredOneSkills.erase(skillIndex); }

        void setRequiredAllSkillMinWeight(const axis_size_t skillIndex, const float minWeight) {
            m_RequiredAllSkills[skillIndex] = minWeight;
        }

        void setRequiredOneSkillMinWeight(const axis_size_t skillIndex, const float minWeight) {
            m_RequiredOneSkills[skillIndex] = minWeight;
        }

        void addRequiredAllSkill(const axis_size_t skillIndex, const float minWeight) {
            m_RequiredAllSkills[skillIndex] = minWeight;
        }

        void addRequiredOneSkill(const axis_size_t skillIndex, const float minWeight) {
            m_RequiredOneSkills[skillIndex] = minWeight;
        }

    private:
        const axis_size_t m_Index;
        const uint8_t m_WeekdayBitMask;
        const Time::DailyInterval m_Interval;
        const std::string m_Name;
        const uint8_t m_SlotCount;
        const uint8_t m_RequiredSlotCount;
        const Time::day_minutes_t m_RestMinutesBefore, m_RestMinutesAfter;

        std::unordered_map<axis_size_t, float> m_RequiredAllSkills {};
        std::unordered_map<axis_size_t, float> m_RequiredOneSkills {};
    };
}

#endif //SHIFT_H

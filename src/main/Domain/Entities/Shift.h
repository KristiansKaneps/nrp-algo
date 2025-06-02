#ifndef SHIFT_H
#define SHIFT_H

#include <string>
#include <unordered_map>
#include <utility>
#include <set>

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
              const Time::day_minutes_t restMinutesBefore, const Time::day_minutes_t restMinutesAfter,
              const Time::day_minutes_t consecutiveRestMinutes) : m_Index(index),
                                                                  m_WeekdayBitMask(weekdayBitMask),
                                                                  m_Interval(interval),
                                                                  m_Name(name),
                                                                  m_SlotCount(slotCount),
                                                                  m_RequiredSlotCount(requiredSlotCount),
                                                                  m_RestMinutesBefore(restMinutesBefore),
                                                                  m_RestMinutesAfter(restMinutesAfter),
                                                                  m_ConsecutiveRestMinutes(consecutiveRestMinutes) { }

        Shift(const axis_size_t index, const uint8_t weekdayBitMask, const Time::DailyInterval& interval,
              const std::string& name, const uint8_t slotCount, const uint8_t requiredSlotCount,
              const Time::day_minutes_t restMinutesBefore, const Time::day_minutes_t restMinutesAfter) : m_Index(index),
            m_WeekdayBitMask(weekdayBitMask),
            m_Interval(interval),
            m_Name(name),
            m_SlotCount(slotCount),
            m_RequiredSlotCount(requiredSlotCount),
            m_RestMinutesBefore(restMinutesBefore),
            m_RestMinutesAfter(restMinutesAfter),
            m_ConsecutiveRestMinutes(calculateDefaultConsecutiveRestMinutes()) { }


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
        [[nodiscard]] uint8_t slotCount(const axis_size_t dayIndex) const {
            return m_SlotCountPerDayIndex.contains(dayIndex) ? m_SlotCountPerDayIndex.at(dayIndex) : m_SlotCount;
        }
        [[nodiscard]] uint8_t requiredSlotCount(const axis_size_t dayIndex) const {
            return m_RequiredSlotCountPerDayIndex.contains(dayIndex)
                       ? m_RequiredSlotCountPerDayIndex.at(dayIndex)
                       : m_RequiredSlotCount;
        }
        [[nodiscard]] Time::day_minutes_t restMinutesBefore() const { return m_RestMinutesBefore; }
        [[nodiscard]] Time::day_minutes_t restMinutesAfter() const { return m_RestMinutesAfter; }
        [[nodiscard]] Time::day_minutes_t consecutiveRestMinutes() const { return m_ConsecutiveRestMinutes; }
        [[nodiscard]] const std::set<axis_size_t>& blockedNextDayShiftIndices() const { return m_BlockedNextDayShiftIndices; }

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

        [[nodiscard]] bool blocksShiftIndex(const axis_size_t skillIndex) const {
            return m_BlockedNextDayShiftIndices.contains(skillIndex);
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

        void setSlotCountAtDay(const axis_size_t dayIndex, const uint8_t slotCount, const uint8_t requiredSlotCount) {
            m_SlotCountPerDayIndex.insert({dayIndex, slotCount});
            m_RequiredSlotCountPerDayIndex.insert({dayIndex, requiredSlotCount});
        }
        void setSlotCountAtDay(const axis_size_t dayIndex, const uint8_t requiredSlotCount) {
            setSlotCountAtDay(dayIndex, requiredSlotCount, requiredSlotCount);
        }

        void addBlockedNextDayShiftIndex(const axis_size_t shiftIndex) {
            m_BlockedNextDayShiftIndices.insert(shiftIndex);
        }
        void removeBlockedNextDayShiftIndex(const axis_size_t shiftIndex) {
            m_BlockedNextDayShiftIndices.erase(shiftIndex);
        }

    protected:
        const axis_size_t m_Index;
        const uint8_t m_WeekdayBitMask;
        const Time::DailyInterval m_Interval;
        const std::string m_Name;
        const uint8_t m_SlotCount;
        const uint8_t m_RequiredSlotCount;
        const Time::day_minutes_t m_RestMinutesBefore, m_RestMinutesAfter;
        const Time::day_minutes_t m_ConsecutiveRestMinutes;

        std::unordered_map<axis_size_t, float> m_RequiredAllSkills {};
        std::unordered_map<axis_size_t, float> m_RequiredOneSkills {};

        std::unordered_map<axis_size_t, uint8_t> m_SlotCountPerDayIndex {};
        std::unordered_map<axis_size_t, uint8_t> m_RequiredSlotCountPerDayIndex {};

        std::set<axis_size_t> m_BlockedNextDayShiftIndices {};

        [[nodiscard]] Time::day_minutes_t calculateDefaultConsecutiveRestMinutes() const {
            const Time::day_minutes_t minutesUntilDayEnd = (1 + (m_Interval.endInMinutes() - 1) / (24 * 60)) * 24 * 60 - m_Interval.endInMinutes(); // NOLINT(*-narrowing-conversions)
            return static_cast<Time::day_minutes_t>(24 * 60 + minutesUntilDayEnd);
        }
    };
}

#endif //SHIFT_H

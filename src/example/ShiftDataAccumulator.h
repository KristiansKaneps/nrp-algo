#ifndef SHIFTDATAACCUMULATOR_H
#define SHIFTDATAACCUMULATOR_H

#include <ranges>
#include <string>
#include <set>
#include <unordered_map>

#include "Time/Range.h"
#include "Time/DailyInterval.h"

namespace ShiftData {
    typedef uint32_t shift_color_t;

    class Skill {
    public:
        std::string m_SkillId;
        float m_Weight;

        Skill(std::string skillId, const float weight) : m_SkillId(std::move(skillId)), m_Weight(weight) { }
    };

    struct ShiftTemplate {
        std::string _shiftId;
        uint8_t slots = 1;

        std::string summary;
        float weight;
        shift_color_t color;
        Time::DailyInterval interval;

        std::vector<Skill> skills;
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

        [[nodiscard]] std::set<std::string> ids() const {
            return m_Ids;
        }

        std::vector<ShiftTemplate> templates() const {
            std::vector<ShiftTemplate> result;
            for (const auto& val : m_ShiftTemplateMap | std::views::values) {
                ShiftTemplate shiftTemplate = val;
                const auto shiftSkills = m_SkillMap.find(val._shiftId);
                shiftTemplate.skills = shiftSkills == m_SkillMap.end() ? std::vector<Skill>() : shiftSkills->second;
                result.emplace_back(shiftTemplate);
            }
            return result;
        }

        void addShift(const std::string& id, const std::string& summary, const float weight, const shift_color_t color, const Time::Range &range, const std::chrono::time_zone *const timeZone, const bool locked) {
            m_Ids.insert(id);
            m_SummaryMap.insert({id, summary});
            m_WeightMap.insert({id, weight});
            m_ColorMap.insert({id, color});
            m_RangeMap.insert({id, range});
            m_LockedMap.insert({id, locked});

            m_ShiftCountPerRangeMap.insert_or_assign(range, m_ShiftCountPerRangeMap.contains(range) ? m_ShiftCountPerRangeMap.at(range) + 1 : 1);
            const uint32_t slots = m_ShiftCountPerRangeMap.at(range);

            // ReSharper disable once CppTooWideScopeInitStatement
            const Time::DailyInterval interval = Time::DailyInterval::fromRange(range, timeZone);
            if (!m_ShiftTemplateMap.contains(interval)) {
                const ShiftTemplate shiftTemplate = {
                    ._shiftId = id,
                    .slots = static_cast<uint8_t>(slots),
                    .summary = summary,
                    .weight = weight,
                    .color = color,
                    .interval = interval,
                };
                m_ShiftTemplateMap.insert({interval, shiftTemplate});
            } else {
                // ReSharper disable once CppTooWideScopeInitStatement
                auto &shiftTemplate = m_ShiftTemplateMap.at(interval);
                if (shiftTemplate.slots < slots) {
                    shiftTemplate.slots = static_cast<uint8_t>(slots);
                }
            }
        }

        void addShiftAssignedUserId(const std::string &id, const std::string &userId) {
            m_UserIdMap.insert({id, userId});
        }

        void addShiftAssignedSkillId(const std::string &id, const std::string &skillId) {
            m_AssignedSkillIdMap.insert({id, skillId});
        }

        void addSkills(const std::string &id, const std::vector<Skill>& skills) {
            m_SkillMap.insert({id, skills});
        }

    private:
        std::set<std::string> m_Ids;

        std::unordered_map<std::string, std::string> m_SummaryMap;
        std::unordered_map<std::string, float> m_WeightMap;
        std::unordered_map<std::string, shift_color_t> m_ColorMap;
        std::unordered_map<std::string, Time::Range> m_RangeMap;
        std::unordered_map<std::string, bool> m_LockedMap;
        std::unordered_map<std::string, std::string> m_UserIdMap;
        std::unordered_map<std::string, std::string> m_AssignedSkillIdMap;

        std::unordered_map<std::string, std::vector<Skill>> m_SkillMap;

        std::unordered_map<Time::DailyInterval, ShiftTemplate> m_ShiftTemplateMap;
        std::unordered_map<Time::Range, uint32_t> m_ShiftCountPerRangeMap;
    };
}

#endif //SHIFTDATAACCUMULATOR_H

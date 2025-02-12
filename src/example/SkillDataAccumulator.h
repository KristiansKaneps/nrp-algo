#ifndef SKILLDATAACCUMULATOR_H
#define SKILLDATAACCUMULATOR_H

#include <string>
#include <set>
#include <unordered_map>

namespace SkillData {
    struct Skill {
        std::string id;
        std::string name;
        bool complementary;
    };

    class Accumulator {
    public:
        Accumulator() = default;
        ~Accumulator() = default;

        [[nodiscard]] size_t size() const {
            return m_Ids.size();
        }

        Skill operator[](const size_t index) const {
            auto iterator = m_Ids.begin();
            std::advance(iterator, index);
            const auto &id = *iterator;
            return {
                id,
                m_NameMap.at(id),
                m_ComplementaryMap.at(id),
            };
        }

        [[nodiscard]] size_t getIndex(const std::string& id) const {
            size_t i = 0;
            for(auto it = m_Ids.begin(); it != m_Ids.end(); ++i, ++it) {
                // ReSharper disable once CppTooWideScopeInitStatement
                const auto &skillId = *it;
                if (skillId == id) {
                    return i;
                }
            }
            return -1;
        }

        [[nodiscard]] std::set<std::string> ids() const {
            return m_Ids;
        }

        void retainValidSkills(const std::set<std::string> &skillIds) {
            for(auto it = m_Ids.begin(); it != m_Ids.end();) {
                // ReSharper disable once CppTooWideScopeInitStatement
                const auto &skillId = *it;
                if (!skillIds.contains(skillId)) {
                    m_Ids.erase(it);
                    continue;
                }

                ++it;
            }
        }

        void addSkill(const std::string& id, const std::string& name, const bool complementary) {
            m_Ids.insert(id);
            m_NameMap.insert({id, name});
            m_ComplementaryMap.insert({id, complementary});
        }

    private:
        std::set<std::string> m_Ids;

        std::unordered_map<std::string, std::string> m_NameMap;
        std::unordered_map<std::string, bool> m_ComplementaryMap;
    };
}

#endif //SKILLDATAACCUMULATOR_H

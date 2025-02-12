#ifndef SKILLDATAACCUMULATOR_H
#define SKILLDATAACCUMULATOR_H

#include <string>
#include <unordered_map>
#include <set>

namespace SkillData {
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

        bool complementary(const size_t index) const {
            return m_ComplementaryMap.at(operator[](index));
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

#ifndef RANDOM_H
#define RANDOM_H

#include <random>

namespace Random {
    class RandomGenerator {
    public:
        [[nodiscard]] static RandomGenerator& instance() {
            thread_local RandomGenerator instance;
            return instance;
        }

        RandomGenerator(const RandomGenerator&) = delete;
        RandomGenerator& operator=(const RandomGenerator&) = delete;

        [[nodiscard]] uint32_t randomInt(const uint32_t min, const uint32_t max) {
            std::uniform_int_distribution<std::mt19937::result_type> dist(min, max);
            return dist(m_Rng);
        }

        [[nodiscard]] uint32_t randomInt(const uint32_t max) {
            return randomInt(0, max);
        }

        [[nodiscard]] float randomFloat(const float min, const float max) {
            std::uniform_real_distribution<float> dist(min, max);
            return dist(m_Rng);
        }

        [[nodiscard]] float randomFloat(const float max) {
            return randomFloat(0, max);
        }

        template<typename T>
        [[nodiscard]] T choice(const std::vector<T>& choices) {
            assert(!choices.empty() && "Empty choices vector");
            return choices[randomInt(0, choices.size() - 1)];
        }

    private:
        RandomGenerator() : m_Rng(std::mt19937(std::random_device{}())) { }

        std::mt19937 m_Rng;
    };

    [[nodiscard]] inline RandomGenerator& generator() {
        return RandomGenerator::instance();
    }
}

#endif //RANDOM_H

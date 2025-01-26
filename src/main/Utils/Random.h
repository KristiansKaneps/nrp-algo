#ifndef RANDOM_H
#define RANDOM_H

#include <random>

namespace Random {
    class RandomGenerator {
    public:
        RandomGenerator() : m_Rng(std::mt19937(std::random_device{}())) {
        }
        ~RandomGenerator() = default;

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

    private:
        std::mt19937 m_Rng;
    };
}

#endif //RANDOM_H

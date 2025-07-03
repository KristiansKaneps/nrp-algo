#ifndef RANDOM_H
#define RANDOM_H

#include <random>

namespace Random {
    class RandomGenerator {
    public:
        [[nodiscard]] static RandomGenerator& instance() noexcept {
            thread_local RandomGenerator instance;
            return instance;
        }

        RandomGenerator(const RandomGenerator&) = delete;
        RandomGenerator& operator=(const RandomGenerator&) = delete;

        [[nodiscard]] uint32_t randomInt(const uint32_t min, const uint32_t max) noexcept {
            std::uniform_int_distribution<std::mt19937::result_type> dist(min, max);
            return dist(m_Rng);
        }

        [[nodiscard]] uint32_t randomInt(const uint32_t max) noexcept {
            return randomInt(0, max);
        }

        [[nodiscard]] float randomFloat(const float min, const float max) noexcept {
            std::uniform_real_distribution<float> dist(min, max);
            return dist(m_Rng);
        }

        [[nodiscard]] float randomFloat(const float max) noexcept {
            return randomFloat(0, max);
        }

        template<typename T>
        [[nodiscard]] T choice(const std::vector<T>& choices) noexcept {
            assert(!choices.empty() && "Empty choices vector");
            return choices[randomInt(0, choices.size() - 1)];
        }

    private:
        RandomGenerator() noexcept : m_Rng(std::mt19937(std::random_device{}())) { }

        std::mt19937 m_Rng;
    };

    [[nodiscard]] inline RandomGenerator& generator() noexcept {
        return RandomGenerator::instance();
    }
}

#endif //RANDOM_H

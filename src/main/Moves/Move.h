#ifndef MOVE_H
#define MOVE_H

namespace Moves {
    enum Type {

    };

    class Move {
    public:
        explicit Move(const size_t index) : m_Index(index) {}
        virtual ~Move() = default;

        [[nodiscard]] size_t index() const { return m_Index; }
    private:
        /**
         * Can also be used as an identifier.
         */
        size_t m_Index{};
    };
}

#endif //MOVE_H

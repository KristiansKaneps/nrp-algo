#ifndef NRPPROBLEMSERIALIZER_H
#define NRPPROBLEMSERIALIZER_H

#include "Domain/State/DomainState.h"

#include "IO/StateFile.h"

namespace NrpProblemInstances {
    class NrpProblemSerializer {
    public:
        enum Type : uint8_t {
            TABBED = 0,
            XML,
        };

        explicit NrpProblemSerializer(const Type type = TABBED) : m_Type(type) { }
        ~NrpProblemSerializer() = default;

        void serialize(IO::StateFile& out, const Domain::State::DomainState& state) {
            if (m_Type == TABBED) serializeTabbed(out, state);
            if (m_Type == XML) serializeXml(out, state);
        }

    protected:
        const Type m_Type;

        void serializeTabbed(IO::StateFile& out, const Domain::State::DomainState& state);
        void serializeXml(IO::StateFile& out, const Domain::State::DomainState& state);
    };
}


#endif //NRPPROBLEMSERIALIZER_H

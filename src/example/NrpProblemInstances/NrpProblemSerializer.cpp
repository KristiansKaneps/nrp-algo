#include "NrpProblemSerializer.h"

#include <iostream>
#include <sstream>
#include <vector>

#define HEADER_XML_VERSION "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
#define TAG_XML_ROSTER_BEGIN "<Roster xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"Roster.xsd\">\n"
#define TAG_XML_ROSTER_END "</Roster>\n"

namespace NrpProblemInstances {
    void NrpProblemSerializer::serializeTabbed(IO::StateFile& out, const Domain::State::DomainState& state) {
        for (::State::axis_size_t y = 0; y < state.sizeY(); ++y) {
            for (::State::axis_size_t z = 0; z < state.sizeZ(); ++z) {
                for (::State::axis_size_t x = 0; x < state.sizeX(); ++x) {
                    if (!state.get(x, y, z)) {
                        out << '\t';
                    } else {
                        const auto& shift = state.x()[x];
                        out << shift.name();
                    }

                }
            }
            out << '\n';
        }
    }

    // TODO: Implement `serializeXml`.
    void NrpProblemSerializer::serializeXml(IO::StateFile& out, const Domain::State::DomainState& state) {
        out << HEADER_XML_VERSION;
        out << TAG_XML_ROSTER_BEGIN;
        {

        }
        out << TAG_XML_ROSTER_END;
    }
}

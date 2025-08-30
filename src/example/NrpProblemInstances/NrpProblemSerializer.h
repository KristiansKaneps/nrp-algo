#ifndef NRPPROBLEMSERIALIZER_H
#define NRPPROBLEMSERIALIZER_H

#include "Domain/State/DomainState.h"

#include "IO/StateFile.h"

#include <exception>
#include <unordered_map>

namespace NrpProblemInstances {
    using axis_size_t = ::State::axis_size_t;

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

    class NrpProblemTxtConverter {
    public:
        explicit NrpProblemTxtConverter(const Domain::State::DomainState& state)
            : m_State(state) { }

        void convert(const std::string_view& inputFilePath) noexcept {
            const std::filesystem::path dir = std::filesystem::current_path();
            std::ifstream srcFile { dir / inputFilePath };
            std::ofstream dstFile { dir / (std::string(inputFilePath) + "_converted") };

            if (!srcFile.is_open()) {
                std::cerr << "Error: Could not open source file: " << inputFilePath << std::endl;
                return;
            }
            if (!dstFile.is_open()) {
                std::cerr << "Error: Could not open destination file: " << inputFilePath << "_converted" << std::endl;
                return;
            }

            m_State.clearAll();

            const axis_size_t shiftCount = m_State.sizeX();
            const axis_size_t employeeCount = m_State.sizeY();
            const axis_size_t dayCount = m_State.sizeZ();
            const axis_size_t skillCount = m_State.sizeW();

            std::unordered_map<std::string, axis_size_t> shiftNameToIndexMap{};
            for (axis_size_t x = 0; x < shiftCount; ++x) {
                const auto& s = m_State.x()[x];
                shiftNameToIndexMap.insert({ s.name(), s.index() });
            }

            try {
                size_t employeeIndex = 0;
                std::string line;
                while (std::getline(srcFile, line)) {
                    axis_size_t dayIndex = 0;
                    axis_size_t shiftIndex = 0;
                    for (const char ch : line) {
                        if (shiftIndex == shiftCount) {
                            dayIndex++;
                            shiftIndex = 0;
                        }
                        m_State.assign(shiftIndex, employeeIndex, dayIndex, 0, ch != '\t');
                        shiftIndex++;
                    }
                    employeeIndex++;
                }

                for (axis_size_t y = 0; y < employeeCount; ++y) {
                    for (axis_size_t z = 0; z < dayCount; ++z) {
                        std::string shiftName = "\t";
                        for (axis_size_t x = 0; x < shiftCount; ++x) {
                            if (m_State.get(x, y, z)) {
                                shiftName = m_State.x()[x].name() + '\t';
                                break;
                            }
                        }
                        dstFile << shiftName;
                    }
                    dstFile << '\n';
                }

                srcFile.close();
                dstFile.close();
            } catch (const std::exception& e) {
                try { srcFile.close(); } catch (const std::exception& _) { /* ignored */ }
                try { dstFile.close(); } catch (const std::exception& _) { /* ignored */ }
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }

    protected:
        Domain::State::DomainState m_State;
    };
}


#endif //NRPPROBLEMSERIALIZER_H

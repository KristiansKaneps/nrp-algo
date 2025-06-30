#ifndef STATEFILE_H
#define STATEFILE_H

#include <filesystem>
#include <fstream>
#include <iostream>
#include <utility>

#include "Utils/StringUtils.h"

#define STATE_FILE_OUTPUT_DIRECTORY "notebooks/state_output/"

namespace IO {
    class StateFile {
    public:
        StateFile(std::string filename, const bool prependTimestamp = true) {
            if (prependTimestamp) {
                filename = String::getTimestampPrefix() + filename;
            }

            const std::filesystem::path parentDir = std::filesystem::current_path().parent_path();
            const std::filesystem::path fullPath = parentDir / STATE_FILE_OUTPUT_DIRECTORY;

            std::filesystem::create_directories(fullPath);

            m_Out = std::ofstream(fullPath / filename);

            if (!m_Out) {
                std::cerr << "Failed to open file at: " << (fullPath / filename) << "\n";
            }
        }

        ~StateFile() {
            m_Out.close();
        }

        std::ofstream& stream() { return m_Out; }

        StateFile& operator<<(const char c) {
            m_Out << c;
            return *this;
        }

        StateFile& operator<<(const int32_t integer) {
            m_Out << integer;
            return *this;
        }

        StateFile& operator<<(const uint32_t integer) {
            m_Out << integer;
            return *this;
        }

        StateFile& operator<<(const int64_t integer) {
            m_Out << integer;
            return *this;
        }

        StateFile& operator<<(const uint64_t integer) {
            m_Out << integer;
            return *this;
        }

        StateFile& operator<<(const char* cStr) {
            m_Out << cStr;
            return *this;
        }

        StateFile& operator<<(const std::string& str) {
            m_Out << str;
            return *this;
        }

    protected:
        std::ofstream m_Out;
    };
}

#endif //STATEFILE_H

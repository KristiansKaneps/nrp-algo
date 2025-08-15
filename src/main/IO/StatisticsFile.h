#ifndef STATISTICSFILE_H
#define STATISTICSFILE_H

#include <filesystem>
#include <fstream>
#include <iostream>

#include "Utils/StringUtils.h"

#define DEFAULT_STATISTICS_FILE_OUTPUT_DIRECTORY "statistics_output/"

namespace IO {
    class StatisticsFile {
    public:
        StatisticsFile(std::string filename, const bool prependTimestamp = true) {
            if (prependTimestamp) {
                filename = String::getTimestampPrefix() + filename;
            }

            const std::filesystem::path dir = std::filesystem::current_path();
            const std::filesystem::path fullPath = dir / DEFAULT_STATISTICS_FILE_OUTPUT_DIRECTORY;

            std::filesystem::create_directories(fullPath);

            m_Out = std::ofstream(fullPath / filename);

            if (!m_Out) {
                std::cerr << "Failed to open file at: " << (fullPath / filename) << std::endl;
            }
        }

        StatisticsFile(const std::filesystem::path& directory, std::string filename, const bool prependTimestamp = true) {
            if (prependTimestamp) {
                filename = String::getTimestampPrefix() + filename;
            }

            std::filesystem::path fullPath;
            if (directory.is_absolute()) {
                fullPath = directory;
            } else {
                fullPath = std::filesystem::current_path() / directory;
            }

            std::filesystem::create_directories(fullPath);

            m_Out = std::ofstream(fullPath / filename);

            if (!m_Out) {
                std::cerr << "Failed to open file at: " << (fullPath / filename) << std::endl;
            }
        }

        ~StatisticsFile() {
            m_Out.close();
        }

        std::ofstream& stream() { return m_Out; }

        StatisticsFile& operator<<(const char c) {
            m_Out << c;
            return *this;
        }

        StatisticsFile& operator<<(const int32_t integer) {
            m_Out << integer;
            return *this;
        }

        StatisticsFile& operator<<(const uint32_t integer) {
            m_Out << integer;
            return *this;
        }

        StatisticsFile& operator<<(const int64_t integer) {
            m_Out << integer;
            return *this;
        }

        StatisticsFile& operator<<(const uint64_t integer) {
            m_Out << integer;
            return *this;
        }

        StatisticsFile& operator<<(const char* cStr) {
            m_Out << cStr;
            return *this;
        }

        StatisticsFile& operator<<(const std::string& str) {
            m_Out << str;
            return *this;
        }

    protected:
        std::ofstream m_Out;
    };
}

#endif //STATISTICSFILE_H

#ifndef STATISTICSFILE_H
#define STATISTICSFILE_H

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <iostream>
#include <utility>

#include "Utils/StringUtils.h"

#define STATISTICS_FILE_OUTPUT_DIRECTORY "notebooks/statistics_output/"

namespace IO {
    inline std::string getTimestampPrefix() {
        const auto now = std::chrono::system_clock::now();
        const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        const std::tm tm = String::localtime_xp(now_time_t);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S_"); // e.g., "2025-06-06_13-42-05_"
        return oss.str();
    }

    class StatisticsFile {
    public:
        StatisticsFile(std::string filename, const bool prependTimestamp = true) {
            if (prependTimestamp) {
                filename = getTimestampPrefix() + filename;
            }

            const std::filesystem::path parentDir = std::filesystem::current_path().parent_path();
            const std::filesystem::path fullPath = parentDir / STATISTICS_FILE_OUTPUT_DIRECTORY;

            std::filesystem::create_directories(fullPath);

            m_Out = std::ofstream(fullPath / filename);

            if (!m_Out) {
                std::cerr << "Failed to open file at: " << (fullPath / filename) << "\n";
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

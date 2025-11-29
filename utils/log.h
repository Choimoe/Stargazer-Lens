#ifdef ERROR
#undef ERROR
#endif
#include <fstream>
#include <mutex>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <stdexcept>

enum class LogLevel { DEBUG, INFO, WARN, ERROR };

class Logger {
public:
    Logger(const std::string& path) : logfile_path(path) {
        open();
    }

    ~Logger() {
        if (ofs.is_open()) ofs.close();
    }

    void debug(const std::string& msg) { log(LogLevel::DEBUG, msg); }
    void info(const std::string& msg)  { log(LogLevel::INFO,  msg); }
    void warn(const std::string& msg)  { log(LogLevel::WARN,  msg); }
    void error(const std::string& msg) { log(LogLevel::ERROR, msg); }

private:
    std::string logfile_path;
    std::ofstream ofs;
    std::mutex mtx;

    void open() {
        std::lock_guard<std::mutex> lock(mtx);
        ofs.open(logfile_path, std::ofstream::out | std::ofstream::app);
        if (!ofs.is_open()) {
            std::cerr << "[Logger] 无法打开日志文件: " << logfile_path << "，将仅在控制台输出错误日志。" << std::endl;
        }
    }

    std::string now_str() {
        using namespace std::chrono;
        auto t = system_clock::now();
        auto tt = system_clock::to_time_t(t);
        std::tm tm{};
#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    const char* level_name(LogLevel l) {
        switch (l) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO";
            case LogLevel::WARN:  return "WARN";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNK";
        }
    }

    void log(LogLevel level, const std::string& msg) {
        std::lock_guard<std::mutex> lock(mtx);
        std::ostringstream line;
        line << "[" << now_str() << "] [" << level_name(level) << "] " << msg << "\n";

        if (ofs.is_open()) {
            ofs << line.str();
            ofs.flush();
        }

        // Only ERROR should also be echoed to console per requirement.
        if (level == LogLevel::ERROR) {
            std::cerr << line.str();
        }
    }
};
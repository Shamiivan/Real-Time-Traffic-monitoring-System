// include/utils/Logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <mutex>
#include <bitset>

class Logger {
public:
    enum class Level {
        DEBUG,    // Detailed debugging
        INFO,     // Normal events
        WARNING,  // Issues that need attention
        ERROR,    // Critical issues
        COUNT     // Used to size the bitset
    };

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    // Enable specific log levels
    void enable(Level level) {
        enabledLevels.set(static_cast<size_t>(level), true);
    }

    // Disable specific log levels
    void disable(Level level) {
        enabledLevels.set(static_cast<size_t>(level), false);
    }

    // Check if a level is enabled
    bool isEnabled(Level level) const {
        return enabledLevels[static_cast<size_t>(level)];
    }

    // Enable all levels
    void enableAll() {
        enabledLevels.set();
    }

    // Disable all levels
    void disableAll() {
        enabledLevels.reset();
    }

    void log(Level level, const std::string tag, const std::string& message) {
        if (isEnabled(level)) {
            std::lock_guard<std::mutex> lock(mutex);
            std::cout << "[" << levelToString(level) << "] " << "[" << tag << "] "
                      << message << std::endl;
        }
    }

private:
    Logger() {
        // By default, enable all except DEBUG
        enableAll();
        disable(Level::DEBUG);
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::bitset<static_cast<size_t>(Level::COUNT)> enabledLevels;
    std::mutex mutex;

    const char* levelToString(Level level) {
        switch (level) {
            case Level::DEBUG:   return "DEBUG";
            case Level::INFO:    return "INFO";
            case Level::WARNING: return "WARNING";
            case Level::ERROR:   return "ERROR";
            default:            return "UNKNOWN";
        }
    }
};

// Convenience macros - now with enabled checks
#define LOG_DEBUG(tag, msg)   \
  if(Logger::getInstance().isEnabled(Logger::Level::DEBUG)) \
    Logger::getInstance().log(Logger::Level::DEBUG, tag, msg)
#define LOG_INFO(tag, msg) \
    if(Logger::getInstance().isEnabled(Logger::Level::INFO)) \
        Logger::getInstance().log(Logger::Level::INFO, tag, msg)
#define LOG_WARNING(tag, msg) \
    if(Logger::getInstance().isEnabled(Logger::Level::WARNING)) \
        Logger::getInstance().log(Logger::Level::WARNING, tag, msg)
#define LOG_ERROR(tag, msg) \
    if(Logger::getInstance().isEnabled(Logger::Level::ERROR)) \
        Logger::getInstance().log(Logger::Level::ERROR, tag, msg)



#endif // LOGGER_H
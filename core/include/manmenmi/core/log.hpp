#pragma once

#include <mutex>
#include <ostream>
#include <string_view>
#include <manmenmi/core/result.hpp>

namespace manmenmi {
enum class LogLevel { debug, info, warning, error };
enum class LogCategory { core, config, window, graphics, test };
[[nodiscard]] std::string_view log_level_name(LogLevel level) noexcept;
[[nodiscard]] Result<LogLevel> parse_log_level(std::string_view name);

// Sink must outlive logger. One logger serializes its writes. Not copyable.
class Logger final {
public:
    explicit Logger(std::ostream& sink, LogLevel minimum = LogLevel::info)
        : sink_(sink), minimum_(minimum) {}
    void write(LogLevel level, LogCategory category, std::string_view message);
private:
    std::ostream& sink_;
    const LogLevel minimum_;
    std::mutex mutex_;
};
} // namespace manmenmi
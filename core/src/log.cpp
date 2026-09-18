#include <manmenmi/core/log.hpp>

namespace manmenmi {
std::string_view log_level_name(LogLevel level) noexcept {
    switch (level) {
    case LogLevel::debug: return "debug";
    case LogLevel::info: return "info";
    case LogLevel::warning: return "warning";
    case LogLevel::error: return "error";
    }
    return "unknown";
}

Result<LogLevel> parse_log_level(std::string_view name) {
    for (const auto level : {LogLevel::debug, LogLevel::info, LogLevel::warning, LogLevel::error}) {
        if (name == log_level_name(level)) { return Result<LogLevel>{level}; }
    }
    return Result<LogLevel>{Error{ErrorCode::invalid_argument, "log_level must be debug|info|warning|error"}};
}

namespace {
std::string_view category_name(LogCategory category) {
    switch (category) {
    case LogCategory::core: return "core";
    case LogCategory::config: return "config";
    case LogCategory::window: return "window";
    case LogCategory::graphics: return "graphics";
    case LogCategory::test: return "test";
    }
    return "unknown";
}
} // namespace

void Logger::write(LogLevel level, LogCategory category, std::string_view message) {
    if (level < minimum_) { return; }
    const std::lock_guard lock{mutex_};
    sink_ << '[' << log_level_name(level) << "][" << category_name(category) << "] ";
    for (const char ch : message) {
        const auto byte = static_cast<unsigned char>(ch);
        if (ch == '\n') { sink_ << "\\n"; }
        else if (ch == '\r') { sink_ << "\\r"; }
        else if (ch == '\t') { sink_ << "\\t"; }
        else if (byte < 32 || byte == 127) { sink_ << '?'; }
        else { sink_ << ch; }
    }
    sink_ << '\n';
    sink_.flush();
}
} // namespace manmenmi
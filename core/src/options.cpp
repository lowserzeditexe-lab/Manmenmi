#include <manmenmi/core/config.hpp>

#include <optional>
#include <set>
#include <string>

namespace manmenmi {
Result<Options> parse_options(std::span<const std::string_view> arguments) {
    Options options;
    std::optional<Backend> backend;
    std::optional<bool> fallback;
    std::optional<LogLevel> level;
    std::optional<std::string> file;
    std::set<std::string_view> seen;
    bool command_set = false;
    auto fail = [](std::string message) {
        return Result<Options>{Error{ErrorCode::invalid_argument, std::move(message)}};
    };
    for (const auto arg : arguments) {
        const auto equal = arg.find('=');
        const auto key = arg.substr(0, equal);
        if (!seen.insert(key).second) { return fail("duplicate option: " + std::string{key}); }
        if (key == "--backend" || key == "--config" || key == "--log-level") {
            if (equal == std::string_view::npos || equal + 1 == arg.size()) {
                return fail("option requires a nonempty =value: " + std::string{key});
            }
            const auto value = arg.substr(equal + 1);
            if (key == "--backend") {
                auto result = parse_backend(value);
                if (!result) { return Result<Options>{result.error()}; }
                backend = result.value();
            } else if (key == "--log-level") {
                auto result = parse_log_level(value);
                if (!result) { return Result<Options>{result.error()}; }
                level = result.value();
            } else {
                file = std::string{value};
            }
        } else if (arg == "--allow-fallback" || arg == "--no-fallback") {
            if (fallback.has_value()) { return fail("conflicting fallback options"); }
            fallback = arg == "--allow-fallback";
        } else {
            Command command;
            if (arg == "--help") { command = Command::help; }
            else if (arg == "--version") { command = Command::version; }
            else if (arg == "--check-config") { command = Command::check_config; }
            else if (arg == "--list-backends") { command = Command::list_backends; }
            else if (arg == "--probe") { command = Command::probe; }
            else { return fail("unknown option: " + std::string{arg}); }
            if (command_set) { return fail("choose only one command"); }
            options.command = command;
            command_set = true;
        }
    }
    if (file) {
        auto result = load_config(std::filesystem::path{*file});
        if (!result) { return Result<Options>{result.error()}; }
        options.config = result.value();
    }
    if (backend) { options.config.backend = *backend; }
    if (fallback.has_value()) { options.config.allow_fallback = *fallback; }
    if (level) { options.config.log_level = *level; }
    return Result<Options>{options};
}
} // namespace manmenmi
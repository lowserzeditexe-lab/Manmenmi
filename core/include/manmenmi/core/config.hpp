#pragma once

#include <filesystem>
#include <span>
#include <string_view>
#include <manmenmi/core/backend.hpp>
#include <manmenmi/core/log.hpp>

namespace manmenmi {
struct Config {
    Backend backend = Backend::automatic;
    bool allow_fallback = false;
    LogLevel log_level = LogLevel::info;
};
enum class Command { probe, help, version, check_config, list_backends };
struct Options {
    Config config;
    Command command = Command::probe;
};
inline constexpr std::size_t max_config_bytes = 65536;

[[nodiscard]] Result<Config> parse_config(std::string_view text);
[[nodiscard]] Result<Config> load_config(const std::filesystem::path& path);
// Precedence: defaults < explicit config file < CLI (independent of argument order).
// No implicit config search, no environment-dependent behavior.
[[nodiscard]] Result<Options> parse_options(std::span<const std::string_view> arguments);
} // namespace manmenmi
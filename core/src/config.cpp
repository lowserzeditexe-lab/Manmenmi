#include <manmenmi/core/config.hpp>

#include <array>
#include <fstream>
#include <set>
#include <string>

namespace manmenmi {
namespace {
std::string_view trim(std::string_view value) {
    const auto begin = value.find_first_not_of(" \t\r");
    if (begin == std::string_view::npos) { return {}; }
    return value.substr(begin, value.find_last_not_of(" \t\r") - begin + 1);
}
Result<Config> invalid(std::size_t line, std::string message) {
    return Result<Config>{Error{ErrorCode::invalid_argument,
        "config line " + std::to_string(line) + ": " + message}};
}
} // namespace

Result<Config> parse_config(std::string_view text) {
    if (text.size() > max_config_bytes) { return invalid(0, "maximum size is 65536 bytes"); }
    if (text.find('\0') != std::string_view::npos) { return invalid(0, "NUL byte is not allowed"); }
    Config config;
    std::set<std::string_view> keys;
    std::size_t line_number = 0;
    while (!text.empty()) {
        ++line_number;
        const auto end = text.find('\n');
        const auto line = trim(text.substr(0, end));
        text = end == std::string_view::npos ? std::string_view{} : text.substr(end + 1);
        if (line.empty() || line.starts_with('#')) { continue; }
        const auto equal = line.find('=');
        if (equal == std::string_view::npos) { return invalid(line_number, "expected key=value"); }
        const auto key = trim(line.substr(0, equal));
        const auto value = trim(line.substr(equal + 1));
        if (!keys.insert(key).second) { return invalid(line_number, "duplicate key: " + std::string{key}); }
        if (key == "schema_version") {
            if (value != "1") { return invalid(line_number, "only schema_version=1 is supported"); }
        } else if (key == "backend") {
            auto result = parse_backend(value);
            if (!result) { return invalid(line_number, result.error().message); }
            config.backend = result.value();
        } else if (key == "allow_fallback") {
            if (value != "true" && value != "false") { return invalid(line_number, "allow_fallback must be true|false"); }
            config.allow_fallback = value == "true";
        } else if (key == "log_level") {
            auto result = parse_log_level(value);
            if (!result) { return invalid(line_number, result.error().message); }
            config.log_level = result.value();
        } else {
            return invalid(line_number, "unknown key: " + std::string{key});
        }
    }
    if (!keys.contains("schema_version")) { return invalid(0, "schema_version=1 is required"); }
    return Result<Config>{config};
}

Result<Config> load_config(const std::filesystem::path& path) {
    std::ifstream input{path, std::ios::binary};
    if (!input) { return Result<Config>{Error{ErrorCode::io_error, "cannot open config file"}}; }
    std::array<char, max_config_bytes + 1> bytes{};
    input.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    if (input.bad()) { return Result<Config>{Error{ErrorCode::io_error, "cannot read config file"}}; }
    return parse_config(std::string_view{bytes.data(), static_cast<std::size_t>(input.gcount())});
}
} // namespace manmenmi
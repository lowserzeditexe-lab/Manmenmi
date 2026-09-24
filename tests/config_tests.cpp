#include "check.hpp"
#include <manmenmi/core/config.hpp>

#include <array>
#include <string>

int main(int argc, char** argv) {
    using namespace manmenmi;
    const auto defaults = parse_options({});
    CHECK(defaults);
    CHECK(defaults.value().config.backend == Backend::automatic);
    CHECK(!defaults.value().config.allow_fallback);
    CHECK(defaults.value().command == Command::probe);
    const auto config = parse_config("# comment\r\n schema_version = 1\r\n backend = opengl\r\nallow_fallback=true\nlog_level=debug");
    CHECK(config);
    CHECK(config.value().backend == Backend::opengl);
    CHECK(config.value().allow_fallback);
    CHECK(config.value().log_level == LogLevel::debug);
    for (const std::string_view text : {"", "# only comment", "schema_version=2", "backend=vulkan",
            "schema_version=1\nwrong=true", "schema_version=1\nbackend=metal", "schema_version=1\nbackend=",
            "schema_version=1\nbackend=vulkan\nbackend=dx12", "schema_version=1\nallow_fallback=yes",
            "schema_version=1\nlog_level=quiet", "schema_version=1\nbackend", "schema_version=1\n=value",
            "schema_version=1\nschema_version=1", "schema_version=1 # inline not supported"}) {
        const auto invalid = parse_config(text);
        CHECK(!invalid);
        CHECK(invalid.error().code == ErrorCode::invalid_argument);
    }
    CHECK(!parse_config(std::string{"schema_version=1\0junk", 21}));
    CHECK(!parse_config(std::string(max_config_bytes + 1, ' ')));
    std::string boundary = "schema_version=1\n#";
    boundary.resize(max_config_bytes, 'x');
    CHECK(parse_config(boundary));
    for (const std::string_view option : {"--backend", "--backend=", "--backend=metal", "--unknown",
            "--log-level=trace", "--config=", "--allow-fallback=true", "--help=yes", "vulkan"}) {
        const std::array args{option};
        CHECK(!parse_options(args));
    }
    for (const auto args : {std::array<std::string_view, 2>{"--allow-fallback", "--no-fallback"},
            {"--backend=vulkan", "--backend=dx12"}, {"--help", "--version"}, {"--help", "--help"}}) {
        CHECK(!parse_options(args));
    }
    CHECK(argc == 2);
    if (argc == 2) {
        const std::string file_option = "--config=" + std::string{argv[1]};
        const std::array<std::string_view, 4> args{"--backend=dx12", file_option, "--no-fallback", "--log-level=error"};
        const auto options = parse_options(args);
        CHECK(options);
        CHECK(options.value().config.backend == Backend::dx12);
        CHECK(!options.value().config.allow_fallback);
        CHECK(options.value().config.log_level == LogLevel::error);
        const std::array<std::string_view, 4> reversed{file_option, "--log-level=error", "--no-fallback", "--backend=dx12"};
        CHECK(parse_options(reversed).value().config.backend == Backend::dx12);
        const std::array<std::string_view, 1> file_only{file_option};
        CHECK(parse_options(file_only).value().config.backend == Backend::opengl);
        CHECK(parse_options(file_only).value().config.allow_fallback);
        const auto missing = load_config(std::string{argv[1]} + ".missing");
        CHECK(!missing);
        CHECK(missing.error().code == ErrorCode::io_error);
    }
    return checks::finish();
}
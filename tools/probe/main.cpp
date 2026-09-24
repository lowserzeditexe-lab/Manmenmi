#include <manmenmi/core/runtime.hpp>

#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace {
constexpr std::string_view help = R"(MANMENMI M0 - native compatibility foundation, not an emulator
Usage: manmenmi_probe [command] [options]
Commands (mutually exclusive):
  --help             Show this help
  --version          Show foundation version
  --check-config     Validate configuration only; never validates graphics
  --list-backends    List implementation state; not hardware capabilities
  --probe            Request runtime (default); exits 3 UNIMPLEMENTED in M0
Options:
  --config=PATH      Explicit schema_version=1 key=value file (max 64 KiB)
  --backend=NAME     auto|vulkan|dx12|opengl (default auto)
  --allow-fallback   Explicitly allow other backend candidates
  --no-fallback      Disable fallback, including when enabled by file
  --log-level=LEVEL  debug|info|warning|error (default info)
Precedence: defaults < config file < CLI. Duplicate/unknown options are errors.
auto without fallback requests Vulkan only. M0 opens no window, renders nothing.
Exit codes: 0 diagnostic success, 2 invalid config/CLI, 3 unimplemented/unavailable,
            4 I/O error, 70 unexpected internal failure.
)";

int report_error(const manmenmi::Error& error, manmenmi::Logger& logger) {
    logger.write(manmenmi::LogLevel::error, manmenmi::LogCategory::core,
        std::string{manmenmi::error_name(error.code)} + ": " + error.message);
    switch (error.code) {
    case manmenmi::ErrorCode::invalid_argument: return 2;
    case manmenmi::ErrorCode::unimplemented:
    case manmenmi::ErrorCode::unavailable: return 3;
    case manmenmi::ErrorCode::io_error: return 4;
    default: return 70;
    }
}
int run(int argc, char** argv) {
    using namespace manmenmi;
    Logger diagnostic{std::cerr};
    std::vector<std::string_view> arguments;
    for (int i = 1; i < argc; ++i) { arguments.emplace_back(argv[i]); }
    auto parsed = parse_options(arguments);
    if (!parsed) { return report_error(parsed.error(), diagnostic); }
    const auto& options = parsed.value();
    Logger logger{std::cerr, options.config.log_level};
    if (options.command == Command::help) { std::cout << help; return 0; }
    if (options.command == Command::version) { std::cout << "MANMENMI " MANMENMI_VERSION " M0\n"; return 0; }
    if (options.command == Command::list_backends) {
        for (const auto& backend : backend_inventory()) {
            std::cout << backend_name(backend.backend) << ": UNIMPLEMENTED (not probed)\n";
        }
        return 0;
    }
    if (options.command == Command::check_config) {
        std::cout << "CONFIG_VALID (configuration only)\nbackend=" << backend_name(options.config.backend)
            << "\nallow_fallback=" << (options.config.allow_fallback ? "true" : "false")
            << "\nlog_level=" << log_level_name(options.config.log_level) << '\n';
        return 0;
    }
    auto runtime = create_runtime(options.config, logger);
    if (!runtime) { return report_error(runtime.error(), logger); }
    return report_error(Error{ErrorCode::internal_error, "M0 invariant: no runtime may be created"}, logger);
}
} // namespace

int main(int argc, char** argv) {
    try { return run(argc, argv); }
    catch (const std::exception& error) {
        manmenmi::Logger logger{std::cerr};
        logger.write(manmenmi::LogLevel::error, manmenmi::LogCategory::core, error.what());
        return 70;
    }
}
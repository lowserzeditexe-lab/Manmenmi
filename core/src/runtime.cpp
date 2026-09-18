#include <manmenmi/core/runtime.hpp>

namespace manmenmi {
Result<std::unique_ptr<Runtime>> create_runtime(const Config& config, Logger& logger) {
    const auto candidates = backend_candidates(config.backend, config.allow_fallback);
    bool first = true;
    for (const auto candidate : candidates) {
        if (!first) {
            // Error severity intentionally preserves fallback audit even at --log-level=error.
            logger.write(LogLevel::error, LogCategory::graphics,
                "explicit fallback attempt backend=" + std::string{backend_name(candidate)});
        }
        logger.write(LogLevel::error, LogCategory::graphics,
            "backend=" + std::string{backend_name(candidate)} + " state=UNIMPLEMENTED (M0; no device probe)");
        first = false;
    }
    return Result<std::unique_ptr<Runtime>>{Error{ErrorCode::unimplemented,
        "M0 has no runtime/window/device provider; no window was opened and no frame was presented"}};
}
} // namespace manmenmi
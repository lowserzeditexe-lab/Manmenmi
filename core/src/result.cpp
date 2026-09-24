#include <manmenmi/core/result.hpp>

namespace manmenmi {
const char* error_name(ErrorCode code) noexcept {
    switch (code) {
    case ErrorCode::ok: return "OK";
    case ErrorCode::invalid_argument: return "INVALID_ARGUMENT";
    case ErrorCode::io_error: return "IO_ERROR";
    case ErrorCode::unsupported_backend: return "UNSUPPORTED_BACKEND";
    case ErrorCode::platform_error: return "PLATFORM_ERROR";
    case ErrorCode::surface_creation_failed: return "SURFACE_CREATION_FAILED";
    case ErrorCode::device_creation_failed: return "DEVICE_CREATION_FAILED";
    case ErrorCode::swapchain_creation_failed: return "SWAPCHAIN_CREATION_FAILED";
    case ErrorCode::resize_failed: return "RESIZE_FAILED";
    case ErrorCode::present_failed: return "PRESENT_FAILED";
    case ErrorCode::device_lost: return "DEVICE_LOST";
    case ErrorCode::out_of_memory: return "OUT_OF_MEMORY";
    case ErrorCode::not_initialized: return "NOT_INITIALIZED";
    case ErrorCode::unavailable: return "UNAVAILABLE";
    case ErrorCode::unimplemented: return "UNIMPLEMENTED";
    case ErrorCode::invalid_state: return "INVALID_STATE";
    case ErrorCode::internal_error: return "INTERNAL_ERROR";
    }
    return "UNKNOWN_ERROR";
}

Status success() { return Status{std::monostate{}}; }
} // namespace manmenmi
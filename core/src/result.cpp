#include <manmenmi/core/result.hpp>

namespace manmenmi {
const char* error_name(ErrorCode code) noexcept {
    switch (code) {
    case ErrorCode::invalid_argument: return "INVALID_ARGUMENT";
    case ErrorCode::io_error: return "IO_ERROR";
    case ErrorCode::unimplemented: return "UNIMPLEMENTED";
    case ErrorCode::unavailable: return "UNAVAILABLE";
    case ErrorCode::invalid_state: return "INVALID_STATE";
    case ErrorCode::internal_error: return "INTERNAL_ERROR";
    }
    return "UNKNOWN_ERROR";
}

Status success() { return Status{std::monostate{}}; }
} // namespace manmenmi
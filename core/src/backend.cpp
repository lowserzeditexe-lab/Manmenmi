#include <manmenmi/core/backend.hpp>

namespace manmenmi {
std::string_view backend_name(Backend backend) noexcept {
    switch (backend) {
    case Backend::automatic: return "auto";
    case Backend::vulkan: return "vulkan";
    case Backend::dx12: return "dx12";
    case Backend::opengl: return "opengl";
    }
    return "unknown";
}

Result<Backend> parse_backend(std::string_view name) {
    for (const auto backend : {Backend::automatic, Backend::vulkan, Backend::dx12, Backend::opengl}) {
        if (name == backend_name(backend)) { return Result<Backend>{backend}; }
    }
    return Result<Backend>{Error{ErrorCode::invalid_argument, "backend must be auto|vulkan|dx12|opengl"}};
}

const std::array<BackendInfo, 3>& backend_inventory() noexcept {
    static constexpr std::array<BackendInfo, 3> inventory{{
        {Backend::vulkan, ImplementationState::unimplemented},
        {Backend::dx12, ImplementationState::unimplemented},
        {Backend::opengl, ImplementationState::unimplemented}}};
    return inventory;
}

std::vector<Backend> backend_candidates(Backend requested, bool allow_fallback) {
    std::vector<Backend> candidates{requested == Backend::automatic ? Backend::vulkan : requested};
    if (allow_fallback) {
        for (const auto& entry : backend_inventory()) {
            if (entry.backend != candidates.front()) { candidates.push_back(entry.backend); }
        }
    }
    return candidates;
}
} // namespace manmenmi
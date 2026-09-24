#pragma once

#include <array>
#include <string_view>
#include <vector>
#include <manmenmi/core/result.hpp>

namespace manmenmi {
enum class Backend { automatic, vulkan, dx12, opengl };
enum class ImplementationState { unimplemented }; // No provider exists in M0.
struct BackendInfo {
    Backend backend;
    ImplementationState state;
};

[[nodiscard]] std::string_view backend_name(Backend backend) noexcept;
[[nodiscard]] Result<Backend> parse_backend(std::string_view name);
[[nodiscard]] const std::array<BackendInfo, 3>& backend_inventory() noexcept;
// A policy plan, NOT a device probe or a promise of backend availability.
[[nodiscard]] std::vector<Backend> backend_candidates(Backend requested, bool allow_fallback);
} // namespace manmenmi
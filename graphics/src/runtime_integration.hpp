#pragma once

#include <manmenmi/graphics/graphics.hpp>

#if defined(__has_include)
#if __has_include(<vulkan/vulkan.h>)
#define MANMENMI_HAS_VULKAN 1
#include <vulkan/vulkan.h>
#endif
#endif

#if defined(_WIN32)
#if defined(__has_include)
#if __has_include(<d3d12.h>)
#define MANMENMI_HAS_DX12 1
#include <d3d12.h>
#include <dxgi.h>
#endif
#endif
#endif

#include <vector>

namespace manmenmi::graphics::detail {
[[nodiscard]] Queue* runtime_queue_for_test(Swapchain& swapchain) noexcept;
[[nodiscard]] bool runtime_vulkan_handles_valid_for_test(const Swapchain& swapchain) noexcept;
[[nodiscard]] bool runtime_dx12_handles_valid_for_test(const Swapchain& swapchain) noexcept;
[[nodiscard]] bool runtime_dx12_render_target_has_triangle_for_test(Swapchain& swapchain) noexcept;
[[nodiscard]] bool runtime_dx12_texture_has_triangle_for_test(
	Texture& texture, Swapchain& swapchain) noexcept;
[[nodiscard]] bool runtime_dx12_buffer_handle_valid_for_test(const Buffer& buffer) noexcept;
[[nodiscard]] bool runtime_dx12_write_buffer_for_test(
	Buffer& buffer,
	const std::vector<std::uint8_t>& data) noexcept;
[[nodiscard]] bool runtime_dx12_command_buffer_handle_valid_for_test(
	const CommandBuffer& command_buffer) noexcept;
[[nodiscard]] bool runtime_dx12_buffer_data_matches_for_test(
	const Buffer& buffer,
	const std::vector<std::uint8_t>& expected) noexcept;
} // namespace manmenmi::graphics::detail

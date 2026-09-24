#include "check.hpp"

#include <manmenmi/core/log.hpp>
#include <manmenmi/graphics/graphics.hpp>

#include "../graphics/src/runtime_integration.hpp"
#include "../window/src/sdl_window.hpp"

#include <SDL3/SDL.h>

#include <iostream>
#include <sstream>
#include <string>

namespace {

void print_step(const std::string& backend, const std::string& step, const std::string& status, const std::string& detail = {}) {
    std::cout << "[M1.3][" << backend << "][REAL] " << step << ' ' << status;
    if (!detail.empty()) {
        std::cout << " reason: " << detail;
    }
    std::cout << '\n';
}

bool require_runtime(bool condition, const std::string& backend, const std::string& step, const std::string& reason) {
    if (condition) {
        print_step(backend, step, "PASS");
        return true;
    }
    print_step(backend, step, "FAIL", reason);
    return false;
}

bool require_unavailable(const std::string& backend, const std::string& step, const std::string& reason) {
    print_step(backend, step, "UNAVAILABLE", reason);
    return true;
}

[[maybe_unused]] bool run_vulkan_runtime_integration() {
    using namespace manmenmi;

    std::ostringstream sink;
    Logger logger{sink, LogLevel::debug};

    auto window_result = manmenmi::window::detail::create_window({"MANMENMI M1.3 Vulkan", 1280U, 720U, true, true}, logger);
    if (!window_result) {
        return require_unavailable("VULKAN", "result", std::string{"SDL window creation unavailable: "}.append(window_result.error().message));
    }
    auto window = std::move(window_result).value();
    const auto token = window->native_surface_token();
    if (token == 0U) {
        return require_runtime(false, "VULKAN", "surface", "SurfaceToken is null");
    }
    print_step("VULKAN", "window", "PASS");
    print_step("VULKAN", "surface", "PASS");

    auto factory = graphics::make_graphics_factory();
    auto device_result = factory->create_device(BackendSelection::vulkan, logger);
    if (!device_result) {
        return require_unavailable("VULKAN", "result", std::string{"Vulkan device unavailable: "}.append(device_result.error().message));
    }
    auto device = std::move(device_result).value();
    if (!device->capabilities().presentable) {
        return require_runtime(false, "VULKAN", "device", "device is not presentable");
    }
    print_step("VULKAN", "device", "PASS");

    auto swapchain_result = factory->create_swapchain(*device, token, 1280U, 720U);
    if (!swapchain_result) {
        return require_unavailable("VULKAN", "result", std::string{"Vulkan swapchain unavailable: "}.append(swapchain_result.error().message));
    }
    auto swapchain = std::move(swapchain_result).value();
    if (!graphics::detail::runtime_vulkan_handles_valid_for_test(*swapchain)) {
        return require_runtime(false, "VULKAN", "swapchain", "native Vulkan handles are not valid");
    }
    print_step("VULKAN", "swapchain", "PASS");

    auto* queue = graphics::detail::runtime_queue_for_test(*swapchain);
    if (queue == nullptr) {
        return require_runtime(false, "VULKAN", "queue", "private queue handle was not exposed");
    }
    print_step("VULKAN", "images", "PASS");

    if (!swapchain->acquire_next_image()) {
        return require_runtime(false, "VULKAN", "acquire", "vkAcquireNextImageKHR failed");
    }
    print_step("VULKAN", "acquire", "PASS");

    if (!queue->submit()) {
        return require_runtime(false, "VULKAN", "submit", "vkQueueSubmit failed");
    }
    print_step("VULKAN", "submit", "PASS");

    if (!queue->present()) {
        return require_runtime(false, "VULKAN", "present", "vkQueuePresentKHR failed");
    }
    print_step("VULKAN", "present", "PASS");

    if (!queue->wait_idle()) {
        return require_runtime(false, "VULKAN", "gpu_wait", "vkDeviceWaitIdle failed");
    }
    print_step("VULKAN", "gpu_wait", "PASS");

    if (!swapchain->resize(960U, 540U)) {
        return require_runtime(false, "VULKAN", "resize/recreate", "resize failed");
    }
    if (!swapchain->recreate()) {
        return require_runtime(false, "VULKAN", "resize/recreate", "recreate failed");
    }
    if (!swapchain->acquire_next_image()) {
        return require_runtime(false, "VULKAN", "second frame", "second acquire failed");
    }
    if (!queue->submit()) {
        return require_runtime(false, "VULKAN", "second frame", "second submit failed");
    }
    if (!queue->present()) {
        return require_runtime(false, "VULKAN", "second frame", "second present failed");
    }
    print_step("VULKAN", "resize/recreate", "PASS");
    print_step("VULKAN", "second frame", "PASS");
    return true;
}

bool run_dx12_runtime_integration() {
    using namespace manmenmi;

    std::ostringstream sink;
    Logger logger{sink, LogLevel::debug};

    auto window_result = manmenmi::window::detail::create_window({"MANMENMI M1.3 DX12", 1280U, 720U, true, true}, logger);
    if (!window_result) {
        return require_unavailable("DX12", "result", std::string{"SDL window creation unavailable: "}.append(window_result.error().message));
    }
    auto window = std::move(window_result).value();
    const auto token = window->native_surface_token();
    if (token == 0U) {
        return require_runtime(false, "DX12", "surface", "SurfaceToken is null");
    }
    print_step("DX12", "window", "PASS");
    print_step("DX12", "surface", "PASS");

    auto factory = graphics::make_graphics_factory();
    auto device_result = factory->create_device(BackendSelection::dx12, logger);
    if (!device_result) {
        return require_unavailable("DX12", "result", std::string{"DX12 device unavailable: "}.append(device_result.error().message));
    }
    auto device = std::move(device_result).value();
    if (!device->capabilities().presentable) {
        return require_runtime(false, "DX12", "device", "device is not presentable");
    }
    print_step("DX12", "device", "PASS");

    auto swapchain_result = factory->create_swapchain(*device, token, 1280U, 720U);
    if (!swapchain_result) {
        return require_unavailable("DX12", "result", std::string{"DX12 swapchain unavailable: "}.append(swapchain_result.error().message));
    }
    auto swapchain = std::move(swapchain_result).value();
    if (!graphics::detail::runtime_dx12_handles_valid_for_test(*swapchain)) {
        return require_runtime(false, "DX12", "swapchain", "native DX12 handles are not valid");
    }
    print_step("DX12", "swapchain", "PASS");

    auto* queue = graphics::detail::runtime_queue_for_test(*swapchain);
    if (queue == nullptr) {
        return require_runtime(false, "DX12", "queue", "private queue handle was not exposed");
    }

    if (!swapchain->acquire_next_image()) {
        return require_runtime(false, "DX12", "acquire", "AcquireNextImage failed");
    }
    print_step("DX12", "acquire", "PASS");

    if (!queue->submit()) {
        return require_runtime(false, "DX12", "submit", "ExecuteCommandLists() did not succeed");
    }
    print_step("DX12", "submit", "PASS");

    if (!queue->present()) {
        return require_runtime(false, "DX12", "present", "Present() failed");
    }
    print_step("DX12", "present", "PASS");

    if (!queue->wait_idle()) {
        return require_runtime(false, "DX12", "gpu_wait", "DX12 wait idle failed");
    }
    print_step("DX12", "gpu_wait", "PASS");

    if (!swapchain->resize(960U, 540U)) {
        return require_runtime(false, "DX12", "resize/recreate", "resize failed");
    }
    if (!swapchain->recreate()) {
        return require_runtime(false, "DX12", "resize/recreate", "recreate failed");
    }
    if (!swapchain->acquire_next_image()) {
        return require_runtime(false, "DX12", "second frame", "second acquire failed");
    }
    if (!queue->submit()) {
        return require_runtime(false, "DX12", "second frame", "second submit failed");
    }
    if (!queue->present()) {
        return require_runtime(false, "DX12", "second frame", "second present failed");
    }
    print_step("DX12", "resize/recreate", "PASS");
    print_step("DX12", "second frame", "PASS");
    return true;
}

} // namespace

int main() {
    int failures = 0;

#if defined(MANMENMI_HAS_VULKAN)
    if (!run_vulkan_runtime_integration()) {
        ++failures;
    }
#else
    print_step("VULKAN", "result", "UNAVAILABLE", "Vulkan headers are not available in this build");
#endif

#if defined(_WIN32) && defined(MANMENMI_HAS_DX12)
    if (!run_dx12_runtime_integration()) {
        ++failures;
    }
#else
    print_step("DX12", "result", "UNAVAILABLE", "DX12 runtime is unavailable in this environment");
#endif

    std::cout << "M1.3 real runtime integration failures=" << failures << '\n';
    return failures == 0 ? 0 : 1;
}

#include "check.hpp"
#include <manmenmi/core/runtime.hpp>
#include <manmenmi/window/window.hpp>
#include <manmenmi/graphics/graphics.hpp>
#include <type_traits>

int main() {
    static_assert(std::is_abstract_v<manmenmi::Runtime>);
    static_assert(std::is_abstract_v<manmenmi::window::Window>);
    static_assert(std::is_abstract_v<manmenmi::graphics::Device>);
    static_assert(std::is_abstract_v<manmenmi::graphics::Queue>);
    static_assert(std::is_abstract_v<manmenmi::graphics::Swapchain>);
    static_assert(std::is_abstract_v<manmenmi::graphics::GraphicsFactory>);
    static_assert(std::has_virtual_destructor_v<manmenmi::Runtime>);
    static_assert(std::has_virtual_destructor_v<manmenmi::window::Window>);
    static_assert(std::has_virtual_destructor_v<manmenmi::graphics::Device>);
    static_assert(std::has_virtual_destructor_v<manmenmi::graphics::Queue>);
    static_assert(std::has_virtual_destructor_v<manmenmi::graphics::Swapchain>);
    const manmenmi::window::WindowConfig config{"future neutral client", 1280, 720, true, true};
    CHECK(config.width == 1280);
    CHECK(config.height == 720);
    const manmenmi::SurfaceToken token = 0x5A5A5A5A5A5A5A5AULL;
    CHECK(token == 0x5A5A5A5A5A5A5A5AULL);
    using QueueSubmitFn = manmenmi::Status (manmenmi::graphics::Queue::*)();
    using QueuePresentFn = manmenmi::Status (manmenmi::graphics::Queue::*)();
    using QueueWaitIdleFn = manmenmi::Status (manmenmi::graphics::Queue::*)();
    using QueueResetFn = manmenmi::Status (manmenmi::graphics::Queue::*)();
    using SwapchainAcquireFn = manmenmi::Status (manmenmi::graphics::Swapchain::*)();
    using SwapchainResizeFn = manmenmi::Status (manmenmi::graphics::Swapchain::*)(std::uint32_t, std::uint32_t);
    using SwapchainRecreateFn = manmenmi::Status (manmenmi::graphics::Swapchain::*)();

    static_assert(std::is_same_v<decltype(static_cast<QueueSubmitFn>(&manmenmi::graphics::Queue::submit)), QueueSubmitFn>);
    static_assert(std::is_same_v<decltype(&manmenmi::graphics::Queue::present), QueuePresentFn>);
    static_assert(std::is_same_v<decltype(&manmenmi::graphics::Queue::wait_idle), QueueWaitIdleFn>);
    static_assert(std::is_same_v<decltype(&manmenmi::graphics::Queue::reset), QueueResetFn>);
    static_assert(std::is_same_v<decltype(&manmenmi::graphics::Swapchain::acquire_next_image), SwapchainAcquireFn>);
    static_assert(std::is_same_v<decltype(&manmenmi::graphics::Swapchain::resize), SwapchainResizeFn>);
    static_assert(std::is_same_v<decltype(&manmenmi::graphics::Swapchain::recreate), SwapchainRecreateFn>);
    static_assert(!std::is_invocable_v<decltype(&manmenmi::graphics::Queue::present), manmenmi::graphics::Queue*, const manmenmi::window::Window&>);
    static_assert(!std::is_invocable_v<decltype(&manmenmi::graphics::Queue::present), manmenmi::graphics::Queue*, const manmenmi::graphics::Swapchain&>);
    return checks::finish();
}
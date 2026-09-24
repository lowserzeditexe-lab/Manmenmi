#include "check.hpp"
#include <manmenmi/graphics/graphics.hpp>
#include <manmenmi/core/log.hpp>

#include "../graphics/src/runtime_integration.hpp"
#include "../window/src/sdl_window.hpp"

#include <sstream>

int main() {
    using namespace manmenmi;

    auto factory = graphics::make_graphics_factory();
    CHECK(factory != nullptr);

    std::ostringstream sink;
    Logger logger{sink, LogLevel::debug};

    auto auto_device = factory->create_device(BackendSelection::automatic, logger);
    CHECK(auto_device.has_value() || auto_device.error().code == ErrorCode::unsupported_backend ||
          auto_device.error().code == ErrorCode::unavailable || auto_device.error().code == ErrorCode::device_creation_failed);

    auto vulkan_device = factory->create_device(BackendSelection::vulkan, logger);
    CHECK(vulkan_device.has_value() || vulkan_device.error().code == ErrorCode::unsupported_backend ||
          vulkan_device.error().code == ErrorCode::unavailable || vulkan_device.error().code == ErrorCode::device_creation_failed);

    auto dx12_device = factory->create_device(BackendSelection::dx12, logger);
    CHECK(dx12_device.has_value() || dx12_device.error().code == ErrorCode::unsupported_backend ||
          dx12_device.error().code == ErrorCode::unavailable || dx12_device.error().code == ErrorCode::device_creation_failed);

      if (dx12_device) {
            auto device = std::move(dx12_device).value();
            auto buffer = device->create_buffer({4096U, BufferUsage::transfer_source});
            CHECK(buffer.has_value());
            CHECK(graphics::detail::runtime_dx12_buffer_handle_valid_for_test(*buffer.value()));

            auto zero_size = device->create_buffer({0U, BufferUsage::transfer_source});
            CHECK(!zero_size.has_value());
            CHECK(zero_size.error().code == ErrorCode::invalid_argument);

            auto invalid_usage = device->create_buffer({4096U, static_cast<BufferUsage>(99)});
            CHECK(!invalid_usage.has_value());
            CHECK(invalid_usage.error().code == ErrorCode::invalid_argument);

            auto command = device->create_command_buffer();
            CHECK(command.has_value());
            CHECK(command.value()->begin());
            CHECK(command.value()->close());
            CHECK(!command.value()->begin());
            CHECK(!command.value()->close());
                  CHECK(!command.value()->reset());

                  auto window_result = window::detail::create_window(
                        {"MANMENMI graphics lifecycle", 640U, 480U, false, false}, logger);
                  CHECK(window_result.has_value());
                  auto window = std::move(window_result).value();
                  auto swapchain = factory->create_swapchain(
                        *device, window->native_surface_token(), 640U, 480U);
                  CHECK(swapchain.has_value());
                  auto* queue = graphics::detail::runtime_queue_for_test(*swapchain.value());
                  CHECK(queue != nullptr);
                  CHECK(queue->submit(*command.value()));
                  CHECK(command.value()->reset());
            CHECK(command.value()->begin());
            CHECK(command.value()->close());
      }

    auto opengl_device = factory->create_device(BackendSelection::opengl, logger);
    CHECK(opengl_device.has_value() || opengl_device.error().code == ErrorCode::unsupported_backend ||
          opengl_device.error().code == ErrorCode::unavailable || opengl_device.error().code == ErrorCode::device_creation_failed);

    if (auto_device) {
        auto device = std::move(auto_device).value();
        CHECK(device->backend() == BackendSelection::automatic ||
              device->backend() == BackendSelection::vulkan ||
              device->backend() == BackendSelection::dx12 ||
              device->backend() == BackendSelection::opengl);
        CHECK(device->capabilities().min_width >= 1U);
        CHECK(device->capabilities().min_height >= 1U);
        const auto idle = device->wait_idle();
        CHECK(idle);
    }

    return checks::finish();
}

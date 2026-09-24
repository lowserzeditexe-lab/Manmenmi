#include "check.hpp"

#include <manmenmi/core/log.hpp>
#include <manmenmi/graphics/graphics.hpp>

#include "../graphics/src/runtime_integration.hpp"
#include "../window/src/sdl_window.hpp"

#include <array>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <vector>

int main() {
    using namespace manmenmi;

    std::ostringstream sink;
    Logger logger{sink, LogLevel::debug};
    auto factory = graphics::make_graphics_factory();
    CHECK(factory != nullptr);

    auto window_result = window::detail::create_window(
        {"MANMENMI M3 Triangle", 640U, 480U, true, true}, logger);
    CHECK(window_result.has_value());
    auto window = std::move(window_result).value();
    CHECK(window->native_surface_token() != 0U);

    auto device_result = factory->create_device(BackendSelection::dx12, logger);
    CHECK(device_result.has_value());
    auto device = std::move(device_result).value();
    CHECK(device->capabilities().presentable);
    CHECK(device->capabilities().render_target_supported);

    auto swapchain_result = factory->create_swapchain(
        *device, window->native_surface_token(), 640U, 480U);
    CHECK(swapchain_result.has_value());
    auto swapchain = std::move(swapchain_result).value();
    CHECK(graphics::detail::runtime_dx12_handles_valid_for_test(*swapchain));
    auto* queue = graphics::detail::runtime_queue_for_test(*swapchain);
    CHECK(queue != nullptr);

    constexpr std::array<float, 18> triangle_vertices{
        0.0F, 0.65F, 1.0F, 0.1F, 0.1F, 1.0F,
        -0.65F, -0.55F, 0.1F, 1.0F, 0.1F, 1.0F,
        0.65F, -0.55F, 0.1F, 0.4F, 1.0F, 1.0F,
    };
    std::vector<std::uint8_t> vertex_data(sizeof(triangle_vertices));
    std::memcpy(vertex_data.data(), triangle_vertices.data(), vertex_data.size());

    auto vertex_buffer_result = device->create_buffer({
        vertex_data.size(), BufferUsage::vertex});
    CHECK(vertex_buffer_result.has_value());
    auto vertex_buffer = std::move(vertex_buffer_result).value();
    CHECK(graphics::detail::runtime_dx12_buffer_handle_valid_for_test(*vertex_buffer));
    CHECK(graphics::detail::runtime_dx12_write_buffer_for_test(*vertex_buffer, vertex_data));

    auto pipeline_result = device->create_triangle_pipeline();
    CHECK(pipeline_result.has_value());
    auto pipeline = std::move(pipeline_result).value();

    auto command_result = device->create_command_buffer();
    CHECK(command_result.has_value());
    auto command = std::move(command_result).value();
    CHECK(swapchain->acquire_next_image());
    CHECK(command->begin());
    CHECK(command->begin_render_pass(*swapchain));
    CHECK(command->set_pipeline(*pipeline));
    CHECK(command->bind_vertex_buffer(*vertex_buffer, 0U));
    CHECK(command->draw(3U, 0U));
    CHECK(command->end_render_pass());
    CHECK(command->close());
    CHECK(queue->submit(*command));
    CHECK(graphics::detail::runtime_dx12_render_target_has_triangle_for_test(*swapchain));
    CHECK(queue->present());
    CHECK(queue->wait_idle());

    return checks::finish();
}

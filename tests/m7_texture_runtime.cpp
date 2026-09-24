#include "check.hpp"

#include <manmenmi/core/log.hpp>
#include <manmenmi/graphics/graphics.hpp>

#include "../graphics/src/runtime_integration.hpp"
#include "../window/src/sdl_window.hpp"

#include <array>
#include <cstdint>
#include <sstream>
#include <vector>

int main() {
    using namespace manmenmi;

    std::ostringstream sink;
    Logger logger{sink, LogLevel::debug};
    auto factory = graphics::make_graphics_factory();
    CHECK(factory != nullptr);
    auto window_result = window::detail::create_window(
        {"MANMENMI M7 Texture", 640U, 480U, false, false}, logger);
    CHECK(window_result.has_value());
    auto window = std::move(window_result).value();
    auto device_result = factory->create_device(BackendSelection::dx12, logger);
    CHECK(device_result.has_value());
    auto device = std::move(device_result).value();
    auto swapchain_result = factory->create_swapchain(
        *device, window->native_surface_token(), 640U, 480U);
    CHECK(swapchain_result.has_value());
    auto swapchain = std::move(swapchain_result).value();
    auto* queue = graphics::detail::runtime_queue_for_test(*swapchain);
    CHECK(queue != nullptr);

    auto texture_result = device->create_texture({640U, 480U,
        TextureFormat::rgba8_unorm, TextureUsage::render_target});
    CHECK(texture_result.has_value());
    auto texture = std::move(texture_result).value();

    constexpr std::array<float, 18> vertices{
        0.0F, 0.65F, 1.0F, 0.1F, 0.1F, 1.0F,
        -0.65F, -0.55F, 0.1F, 1.0F, 0.1F, 1.0F,
        0.65F, -0.55F, 0.1F, 0.4F, 1.0F, 1.0F,
    };
    auto vertex_buffer_result = device->create_buffer({
        sizeof(vertices), BufferUsage::vertex});
    CHECK(vertex_buffer_result.has_value());
    auto vertex_buffer = std::move(vertex_buffer_result).value();
    const auto vertex_bytes = std::span<const std::uint8_t>(
        reinterpret_cast<const std::uint8_t*>(vertices.data()), sizeof(vertices));
    CHECK(vertex_buffer->upload(vertex_bytes));

    auto pipeline_result = device->create_triangle_pipeline();
    CHECK(pipeline_result.has_value());
    auto pipeline = std::move(pipeline_result).value();
    auto command_result = device->create_command_buffer();
    CHECK(command_result.has_value());
    auto command = std::move(command_result).value();

    CHECK(command->begin());
    CHECK(command->begin_render_pass(*texture));
    CHECK(command->set_pipeline(*pipeline));
    CHECK(command->bind_vertex_buffer(*vertex_buffer, 0U));
    CHECK(command->draw(3U, 0U));
    CHECK(command->end_render_pass());
    CHECK(command->close());
    CHECK(queue->submit(*command));
    CHECK(graphics::detail::runtime_dx12_texture_has_triangle_for_test(*texture, *swapchain));
    CHECK(queue->wait_idle());

    return checks::finish();
}

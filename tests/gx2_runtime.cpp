#include "check.hpp"

#include <manmenmi/core/log.hpp>
#include <manmenmi/gx2/gx2.hpp>

#include "../graphics/src/runtime_integration.hpp"
#include "../window/src/sdl_window.hpp"

#include <array>
#include <sstream>

int main() {
    using namespace manmenmi;

    std::ostringstream sink;
    Logger logger{sink, LogLevel::debug};
    auto factory = graphics::make_graphics_factory();
    CHECK(factory != nullptr);

    auto window_result = window::detail::create_window(
        {"MANMENMI M4 GX2", 640U, 480U, true, true}, logger);
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
    CHECK(graphics::detail::runtime_dx12_handles_valid_for_test(*swapchain));

    auto small_buffer_result = device->create_buffer({4U, BufferUsage::vertex});
    CHECK(small_buffer_result.has_value());
    auto small_buffer = std::move(small_buffer_result).value();
    const std::array<std::uint8_t, 5> oversized_data{};
    CHECK(!small_buffer->upload(oversized_data));

    const std::array<gx2::Vertex, 3> vertices{
        gx2::Vertex{0.0F, 0.65F, 1.0F, 0.1F, 0.1F, 1.0F},
        gx2::Vertex{-0.65F, -0.55F, 0.1F, 1.0F, 0.1F, 1.0F},
        gx2::Vertex{0.65F, -0.55F, 0.1F, 0.4F, 1.0F, 1.0F},
    };
    gx2::Context context{*device, *queue, *swapchain};
    const std::array<gx2::Vertex, 2> invalid_vertices{};
    CHECK(!context.draw_triangle(invalid_vertices));
    CHECK(context.draw_triangle(vertices));

    return checks::finish();
}

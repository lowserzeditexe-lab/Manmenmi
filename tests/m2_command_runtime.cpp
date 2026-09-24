#include "check.hpp"

#include <manmenmi/core/log.hpp>
#include <manmenmi/graphics/graphics.hpp>

#include "../graphics/src/runtime_integration.hpp"
#include "../window/src/sdl_window.hpp"

#include <sstream>

int main() {
    using namespace manmenmi;

    std::ostringstream sink;
    Logger logger{sink, LogLevel::debug};
    auto factory = graphics::make_graphics_factory();
    CHECK(factory != nullptr);

    auto window_result = window::detail::create_window(
        {"MANMENMI M2.2", 640U, 480U, false, false}, logger);
    CHECK(window_result.has_value());
    auto window = std::move(window_result).value();
    const auto token = window->native_surface_token();
    CHECK(token != 0U);

    auto device_result = factory->create_device(BackendSelection::dx12, logger);
    CHECK(device_result.has_value());
    auto device = std::move(device_result).value();

    auto source_result = device->create_buffer({4096U, BufferUsage::transfer_source});
    auto destination_result = device->create_buffer({4096U, BufferUsage::transfer_destination});
    CHECK(source_result.has_value());
    CHECK(destination_result.has_value());
    CHECK(graphics::detail::runtime_dx12_buffer_handle_valid_for_test(*source_result.value()));
    CHECK(graphics::detail::runtime_dx12_buffer_handle_valid_for_test(*destination_result.value()));

    auto command_result = device->create_command_buffer();
    CHECK(command_result.has_value());
    auto command_buffer = std::move(command_result).value();
    CHECK(graphics::detail::runtime_dx12_command_buffer_handle_valid_for_test(*command_buffer));
    CHECK(!command_buffer->copy_buffer(*source_result.value(), *destination_result.value(), 0U, 0U, 256U));
    CHECK(command_buffer->begin());
    CHECK(command_buffer->copy_buffer(*source_result.value(), *destination_result.value(), 0U, 0U, 128U));
    CHECK(command_buffer->copy_buffer(*source_result.value(), *destination_result.value(), 128U, 128U, 128U));
    CHECK(command_buffer->close());
    CHECK(!command_buffer->copy_buffer(*source_result.value(), *destination_result.value(), 0U, 0U, 256U));
    CHECK(!command_buffer->close());

    auto swapchain_result = factory->create_swapchain(*device, token, 640U, 480U);
    CHECK(swapchain_result.has_value());
    auto swapchain = std::move(swapchain_result).value();
    auto* queue = graphics::detail::runtime_queue_for_test(*swapchain);
    CHECK(queue != nullptr);

    auto recording_command_result = device->create_command_buffer();
    CHECK(recording_command_result.has_value());
    auto recording_command = std::move(recording_command_result).value();
    CHECK(recording_command->begin());
    CHECK(!queue->submit(*recording_command));
    CHECK(recording_command->close());

    std::vector<std::uint8_t> source_data(256U, 0U);
    for (std::size_t i = 0; i < source_data.size(); ++i) {
        source_data[i] = static_cast<std::uint8_t>((i * 7U + 13U) & 0xFFU);
    }
    std::vector<std::uint8_t> destination_data(256U, 0xAAU);

    CHECK(graphics::detail::runtime_dx12_write_buffer_for_test(*source_result.value(), source_data));
    CHECK(graphics::detail::runtime_dx12_write_buffer_for_test(*destination_result.value(), destination_data));
    CHECK(graphics::detail::runtime_dx12_buffer_data_matches_for_test(*destination_result.value(), destination_data));
    CHECK(queue->submit(*command_buffer));
    CHECK(command_buffer->reset());
    CHECK(command_buffer->begin());
    CHECK(command_buffer->close());
    CHECK(queue->submit(*command_buffer));
    CHECK(queue->wait_idle());
    CHECK(graphics::detail::runtime_dx12_buffer_data_matches_for_test(*destination_result.value(), source_data));

    return checks::finish();
}

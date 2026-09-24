#include "check.hpp"

#include <manmenmi/core/log.hpp>
#include <manmenmi/gx2/gx2.hpp>

#include "../graphics/src/runtime_integration.hpp"
#include "../gx2/src/runtime_integration.hpp"
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
      {"MANMENMI M8 GX2 Texture", 640U, 480U, false, false}, logger);
  CHECK(window_result.has_value());
  auto window = std::move(window_result).value();
  auto device_result = factory->create_device(BackendSelection::dx12, logger);
  CHECK(device_result.has_value());
  auto device = std::move(device_result).value();
  auto swapchain_result = factory->create_swapchain(
      *device, window->native_surface_token(), 640U, 480U);
  CHECK(swapchain_result.has_value());
  auto swapchain = std::move(swapchain_result).value();
  auto *queue = graphics::detail::runtime_queue_for_test(*swapchain);
  CHECK(queue != nullptr);

  gx2::Context context{*device, *queue, *swapchain};
  auto texture_result = context.create_texture(
      {640U, 480U, gx2::TextureFormat::unorm_r8_g8_b8_a8,
       gx2::TextureUsage::color_buffer, gx2::TextureLayout::linear_aligned});
  CHECK(texture_result.has_value());
  auto texture = std::move(texture_result).value();

  const std::array<gx2::Vertex, 3> vertices{
      gx2::Vertex{0.0F, 0.65F, 1.0F, 0.1F, 0.1F, 1.0F},
      gx2::Vertex{-0.65F, -0.55F, 0.1F, 1.0F, 0.1F, 1.0F},
      gx2::Vertex{0.65F, -0.55F, 0.1F, 0.4F, 1.0F, 1.0F},
  };
  CHECK(context.draw_triangle(*texture, vertices));
  CHECK(graphics::detail::runtime_dx12_texture_has_triangle_for_test(
      gx2::detail::texture_for_test(*texture), *swapchain));
  return checks::finish();
}

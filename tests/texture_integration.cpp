#include "check.hpp"

#include <manmenmi/core/log.hpp>
#include <manmenmi/graphics/graphics.hpp>

#include <sstream>

int main() {
  using namespace manmenmi;

  std::ostringstream sink;
  Logger logger{sink, LogLevel::debug};
  auto factory = graphics::make_graphics_factory();
  CHECK(factory != nullptr);
  auto device_result = factory->create_device(BackendSelection::dx12, logger);
  CHECK(device_result.has_value());
  auto device = std::move(device_result).value();

  auto texture_result = device->create_texture(
      {64U, 64U, TextureFormat::rgba8_unorm, TextureUsage::render_target});
  CHECK(texture_result.has_value());
  CHECK(!device->create_texture(
      {0U, 64U, TextureFormat::rgba8_unorm, TextureUsage::render_target}));
  CHECK(!device->create_texture(
      {64U, 0U, TextureFormat::rgba8_unorm, TextureUsage::render_target}));
  CHECK(!device->create_texture(
      {64U, 64U, static_cast<TextureFormat>(99), TextureUsage::render_target}));
  CHECK(!device->create_texture(
      {64U, 64U, TextureFormat::rgba8_unorm, static_cast<TextureUsage>(99)}));
  CHECK(!device->create_texture({64U, 64U, TextureFormat::rgba8_unorm,
                                 TextureUsage::render_target,
                                 static_cast<TextureLayout>(99)}));

  return checks::finish();
}

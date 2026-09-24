#include "check.hpp"

#include <manmenmi/gx2/gx2.hpp>

#include <cstdint>

int main() {
  using namespace manmenmi::gx2;

  CHECK(static_cast<std::uint32_t>(TextureFormat::unorm_r8_g8_b8_a8) == 0x01AU);
  CHECK(static_cast<std::uint32_t>(TextureUsage::color_buffer) == (1U << 1U));
  CHECK(static_cast<std::uint32_t>(TextureLayout::linear_aligned) == 1U);

  const TextureDescriptor descriptor{
      128U, 64U, TextureFormat::unorm_r8_g8_b8_a8, TextureUsage::color_buffer,
      TextureLayout::linear_aligned};
  const auto mapped = map_texture_descriptor(descriptor);
  CHECK(mapped.has_value());
  CHECK(mapped.value().width == 128U);
  CHECK(mapped.value().height == 64U);
  CHECK(mapped.value().format == manmenmi::TextureFormat::rgba8_unorm);
  CHECK(mapped.value().usage == manmenmi::TextureUsage::render_target);
  CHECK(mapped.value().layout == manmenmi::TextureLayout::opaque_render_target);

  const auto mapped_again = map_texture_descriptor(descriptor);
  CHECK(mapped_again.has_value());
  CHECK(mapped_again.value().width == mapped.value().width);
  CHECK(mapped_again.value().height == mapped.value().height);
  CHECK(mapped_again.value().format == mapped.value().format);
  CHECK(mapped_again.value().usage == mapped.value().usage);
  CHECK(mapped_again.value().layout == mapped.value().layout);

  CHECK(!map_texture_descriptor({0U, 64U, TextureFormat::unorm_r8_g8_b8_a8,
                                 TextureUsage::color_buffer,
                                 TextureLayout::linear_aligned}));
  CHECK(!map_texture_descriptor({64U, 0U, TextureFormat::unorm_r8_g8_b8_a8,
                                 TextureUsage::color_buffer,
                                 TextureLayout::linear_aligned}));
  CHECK(!map_texture_descriptor({64U, 64U, static_cast<TextureFormat>(99),
                                 TextureUsage::color_buffer,
                                 TextureLayout::linear_aligned}));
  CHECK(!map_texture_descriptor({64U, 64U, TextureFormat::unorm_r8_g8_b8_a8,
                                 static_cast<TextureUsage>(99),
                                 TextureLayout::linear_aligned}));
  CHECK(!map_texture_descriptor({64U, 64U, TextureFormat::unorm_r8_g8_b8_a8,
                                 TextureUsage::color_buffer,
                                 static_cast<TextureLayout>(99)}));
  return checks::finish();
}

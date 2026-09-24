#include "check.hpp"

#include <manmenmi/graphics/graphics.hpp>

#include <type_traits>

int main() {
  using namespace manmenmi;

  CHECK(std::is_enum_v<TextureFormat>);
  CHECK(std::is_enum_v<TextureUsage>);
  CHECK(std::is_enum_v<TextureLayout>);
  CHECK(TextureFormat::rgba8_unorm == TextureFormat::rgba8_unorm);
  CHECK(TextureUsage::render_target == TextureUsage::render_target);
  CHECK(TextureLayout::opaque_render_target ==
        TextureLayout::opaque_render_target);
  CHECK(TextureDescriptor{}.width == 0U);
  CHECK(TextureDescriptor{}.height == 0U);
  return checks::finish();
}

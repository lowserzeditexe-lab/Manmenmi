#pragma once

#include <cstdint>
#include <memory>
#include <span>

#include <manmenmi/core/result.hpp>
#include <manmenmi/graphics/graphics.hpp>

namespace manmenmi::gx2 {

struct Vertex {
  float x;
  float y;
  float red;
  float green;
  float blue;
  float alpha;
};

enum class TextureFormat {
  // GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8 (WUT S4): 0x01a.
  unorm_r8_g8_b8_a8 = 0x01AU
};

enum class TextureLayout : std::uint32_t {
  // GX2_TILE_MODE_LINEAR_ALIGNED (WUT S4): 1. Physical semantics remain
  // unknown.
  linear_aligned = 1U
};

enum class TextureUsage : std::uint32_t {
  // GX2_SURFACE_USE_COLOR_BUFFER (WUT S4): 1 << 1.
  color_buffer = 1U << 1U
};

struct TextureDescriptor {
  std::uint32_t width = 0U;
  std::uint32_t height = 0U;
  TextureFormat format = TextureFormat::unorm_r8_g8_b8_a8;
  TextureUsage usage = TextureUsage::color_buffer;
  TextureLayout layout = TextureLayout::linear_aligned;
};

[[nodiscard]] Result<::manmenmi::TextureDescriptor>
map_texture_descriptor(const TextureDescriptor &descriptor);

class Texture;
namespace detail {
[[nodiscard]] ::manmenmi::Texture &texture_for_test(Texture &texture) noexcept;
}

class Texture final {
public:
  Texture(Texture &&) noexcept = default;
  Texture &operator=(Texture &&) noexcept = default;
  ~Texture() = default;

  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;

private:
  friend class Context;
  friend ::manmenmi::Texture &
  detail::texture_for_test(Texture &texture) noexcept;

  explicit Texture(std::unique_ptr<::manmenmi::Texture> texture) noexcept;

  std::unique_ptr<::manmenmi::Texture> texture_;
};

class Context final {
public:
  explicit Context(graphics::Device &device) noexcept;
  Context(graphics::Device &device, graphics::Queue &queue,
          graphics::Swapchain &swapchain) noexcept;

  [[nodiscard]] Result<std::unique_ptr<Texture>>
  create_texture(const TextureDescriptor &descriptor);
  [[nodiscard]] Status draw_triangle(std::span<const Vertex> vertices);
  [[nodiscard]] Status draw_triangle(Texture &texture,
                                     std::span<const Vertex> vertices);

private:
  graphics::Device *device_;
  graphics::Queue *queue_;
  graphics::Swapchain *swapchain_;
};

} // namespace manmenmi::gx2

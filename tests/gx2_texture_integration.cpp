#include "check.hpp"

#include <manmenmi/gx2/gx2.hpp>

namespace {

class RecordingTexture final : public manmenmi::Texture {};

class RecordingDevice final : public manmenmi::graphics::Device {
public:
  [[nodiscard]] manmenmi::BackendSelection backend() const noexcept override {
    return manmenmi::BackendSelection::automatic;
  }

  [[nodiscard]] manmenmi::BackendCapabilities
  capabilities() const noexcept override {
    return {};
  }

  manmenmi::Status wait_idle() override { return manmenmi::success(); }

  [[nodiscard]] manmenmi::Result<std::unique_ptr<manmenmi::Buffer>>
  create_buffer(const manmenmi::BufferDescriptor &) override {
    return manmenmi::Result<std::unique_ptr<manmenmi::Buffer>>{
        manmenmi::Error{manmenmi::ErrorCode::unimplemented,
                        "buffer creation is not used by this test"}};
  }

  [[nodiscard]] manmenmi::Result<std::unique_ptr<manmenmi::Texture>>
  create_texture(const manmenmi::TextureDescriptor &descriptor) override {
    received = descriptor;
    create_texture_called = true;
    return manmenmi::Result<std::unique_ptr<manmenmi::Texture>>{
        std::make_unique<RecordingTexture>()};
  }

  [[nodiscard]] manmenmi::Result<std::unique_ptr<manmenmi::CommandBuffer>>
  create_command_buffer() override {
    return manmenmi::Result<std::unique_ptr<manmenmi::CommandBuffer>>{
        manmenmi::Error{manmenmi::ErrorCode::unimplemented,
                        "command creation is not used by this test"}};
  }

  [[nodiscard]] manmenmi::Result<std::unique_ptr<manmenmi::Pipeline>>
  create_triangle_pipeline() override {
    return manmenmi::Result<std::unique_ptr<manmenmi::Pipeline>>{
        manmenmi::Error{manmenmi::ErrorCode::unimplemented,
                        "pipeline creation is not used by this test"}};
  }

  manmenmi::TextureDescriptor received{};
  bool create_texture_called = false;
};

} // namespace

int main() {
  using namespace manmenmi;

  RecordingDevice device;
  gx2::Context context{device};
  auto texture_result = context.create_texture(
      {64U, 32U, gx2::TextureFormat::unorm_r8_g8_b8_a8,
       gx2::TextureUsage::color_buffer, gx2::TextureLayout::linear_aligned});
  CHECK(texture_result.has_value());
  CHECK(device.create_texture_called);
  CHECK(device.received.width == 64U);
  CHECK(device.received.height == 32U);
  CHECK(device.received.format == TextureFormat::rgba8_unorm);
  CHECK(device.received.usage == TextureUsage::render_target);
  CHECK(device.received.layout == TextureLayout::opaque_render_target);
  return checks::finish();
}

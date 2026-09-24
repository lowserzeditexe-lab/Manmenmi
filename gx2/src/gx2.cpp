#include <manmenmi/gx2/gx2.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>

namespace manmenmi::gx2 {

Result<::manmenmi::TextureDescriptor>
map_texture_descriptor(const TextureDescriptor &descriptor) {
  if (descriptor.width == 0U || descriptor.height == 0U) {
    return Result<::manmenmi::TextureDescriptor>{
        Error{ErrorCode::invalid_argument,
              "GX2 texture dimensions must be non-zero"}};
  }
  if (descriptor.format != TextureFormat::unorm_r8_g8_b8_a8) {
    return Result<::manmenmi::TextureDescriptor>{Error{
        ErrorCode::invalid_argument, "GX2 texture format is unsupported"}};
  }
  if (descriptor.usage != TextureUsage::color_buffer) {
    return Result<::manmenmi::TextureDescriptor>{
        Error{ErrorCode::invalid_argument, "GX2 texture usage is unsupported"}};
  }
  if (descriptor.layout != TextureLayout::linear_aligned) {
    return Result<::manmenmi::TextureDescriptor>{Error{
        ErrorCode::invalid_argument, "GX2 texture layout is unsupported"}};
  }
  return Result<::manmenmi::TextureDescriptor>{::manmenmi::TextureDescriptor{
      descriptor.width, descriptor.height,
      ::manmenmi::TextureFormat::rgba8_unorm,
      ::manmenmi::TextureUsage::render_target,
      ::manmenmi::TextureLayout::opaque_render_target}};
}

Texture::Texture(std::unique_ptr<::manmenmi::Texture> texture) noexcept
    : texture_(std::move(texture)) {}

namespace detail {
::manmenmi::Texture &texture_for_test(Texture &texture) noexcept {
  return *texture.texture_;
}
} // namespace detail

Context::Context(graphics::Device &device) noexcept
    : device_(&device), queue_(nullptr), swapchain_(nullptr) {}

Context::Context(graphics::Device &device, graphics::Queue &queue,
                 graphics::Swapchain &swapchain) noexcept
    : device_(&device), queue_(&queue), swapchain_(&swapchain) {}

Result<std::unique_ptr<Texture>>
Context::create_texture(const TextureDescriptor &descriptor) {
  if (device_ == nullptr) {
    return Result<std::unique_ptr<Texture>>{
        Error{ErrorCode::not_initialized, "GX2 context is not initialized"}};
  }
  const auto mapped = map_texture_descriptor(descriptor);
  if (!mapped) {
    return Result<std::unique_ptr<Texture>>{mapped.error()};
  }
  auto texture = device_->create_texture(mapped.value());
  if (!texture) {
    return Result<std::unique_ptr<Texture>>{texture.error()};
  }
  return Result<std::unique_ptr<Texture>>{
      std::unique_ptr<Texture>{new Texture(std::move(texture).value())}};
}

Status Context::draw_triangle(std::span<const Vertex> vertices) {
  if (device_ == nullptr || queue_ == nullptr || swapchain_ == nullptr) {
    return Status{
        Error{ErrorCode::not_initialized, "GX2 context is not initialized"}};
  }
  if (vertices.size() != 3U) {
    return Status{Error{ErrorCode::invalid_argument,
                        "GX2 minimal draw requires three vertices"}};
  }

  auto vertex_buffer_result =
      device_->create_buffer({vertices.size_bytes(), BufferUsage::vertex});
  if (!vertex_buffer_result) {
    return Status{vertex_buffer_result.error()};
  }
  auto vertex_buffer = std::move(vertex_buffer_result).value();
  const auto vertex_bytes = std::span<const std::uint8_t>(
      reinterpret_cast<const std::uint8_t *>(vertices.data()),
      vertices.size_bytes());
  auto upload_status = vertex_buffer->upload(vertex_bytes);
  if (!upload_status) {
    return upload_status;
  }

  auto pipeline_result = device_->create_triangle_pipeline();
  if (!pipeline_result) {
    return Status{pipeline_result.error()};
  }
  auto pipeline = std::move(pipeline_result).value();

  auto command_result = device_->create_command_buffer();
  if (!command_result) {
    return Status{command_result.error()};
  }
  auto command = std::move(command_result).value();

  if (auto status = swapchain_->acquire_next_image(); !status) {
    return status;
  }
  if (auto status = command->begin(); !status) {
    return status;
  }
  if (auto status = command->begin_render_pass(*swapchain_); !status) {
    return status;
  }
  if (auto status = command->set_pipeline(*pipeline); !status) {
    return status;
  }
  if (auto status = command->bind_vertex_buffer(*vertex_buffer, 0U); !status) {
    return status;
  }
  if (auto status = command->draw(3U, 0U); !status) {
    return status;
  }
  if (auto status = command->end_render_pass(); !status) {
    return status;
  }
  if (auto status = command->close(); !status) {
    return status;
  }
  if (auto status = queue_->submit(*command); !status) {
    return status;
  }
  if (auto status = queue_->present(); !status) {
    return status;
  }
  return queue_->wait_idle();
}

Status Context::draw_triangle(Texture &texture,
                              std::span<const Vertex> vertices) {
  if (device_ == nullptr || queue_ == nullptr) {
    return Status{
        Error{ErrorCode::not_initialized, "GX2 context is not initialized"}};
  }
  if (vertices.size() != 3U) {
    return Status{Error{ErrorCode::invalid_argument,
                        "GX2 minimal draw requires three vertices"}};
  }
  auto vertex_buffer_result =
      device_->create_buffer({vertices.size_bytes(), BufferUsage::vertex});
  if (!vertex_buffer_result) {
    return Status{vertex_buffer_result.error()};
  }
  auto vertex_buffer = std::move(vertex_buffer_result).value();
  const auto vertex_bytes = std::span<const std::uint8_t>(
      reinterpret_cast<const std::uint8_t *>(vertices.data()),
      vertices.size_bytes());
  if (auto status = vertex_buffer->upload(vertex_bytes); !status) {
    return status;
  }

  auto pipeline_result = device_->create_triangle_pipeline();
  if (!pipeline_result) {
    return Status{pipeline_result.error()};
  }
  auto pipeline = std::move(pipeline_result).value();
  auto command_result = device_->create_command_buffer();
  if (!command_result) {
    return Status{command_result.error()};
  }
  auto command = std::move(command_result).value();
  if (auto status = command->begin(); !status) {
    return status;
  }
  if (auto status = command->begin_render_pass(*texture.texture_); !status) {
    return status;
  }
  if (auto status = command->set_pipeline(*pipeline); !status) {
    return status;
  }
  if (auto status = command->bind_vertex_buffer(*vertex_buffer, 0U); !status) {
    return status;
  }
  if (auto status = command->draw(3U, 0U); !status) {
    return status;
  }
  if (auto status = command->end_render_pass(); !status) {
    return status;
  }
  if (auto status = command->close(); !status) {
    return status;
  }
  if (auto status = queue_->submit(*command); !status) {
    return status;
  }
  return queue_->wait_idle();
}

} // namespace manmenmi::gx2

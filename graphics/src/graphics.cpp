#include <manmenmi/graphics/graphics.hpp>

#include "shader_ir.hpp"

#include "../../window/src/sdl_window.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#if defined(__has_include)
#if __has_include(<vulkan/vulkan.h>)
#define MANMENMI_HAS_VULKAN 1
#include <vulkan/vulkan.h>
#endif
#endif

#if defined(_WIN32)
#if defined(__has_include)
#if __has_include(<d3d12.h>)
#define MANMENMI_HAS_DX12 1
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_4.h>
#include <windows.h>
#endif
#endif
#endif

namespace {
struct ResolvedSurface {
  bool valid = false;
  SDL_Window *window = nullptr;
};

ResolvedSurface
resolve_surface_token(const manmenmi::SurfaceToken &surface_token) noexcept {
  if (surface_token == 0U) {
    return {};
  }

  auto *window = manmenmi::window::detail::resolve_surface_token(surface_token);
  return {window != nullptr, window};
}
} // namespace

namespace manmenmi {
namespace graphics {
namespace detail {

constexpr std::string_view backend_name(BackendSelection backend) noexcept {
  switch (backend) {
  case BackendSelection::automatic:
    return "auto";
  case BackendSelection::vulkan:
    return "vulkan";
  case BackendSelection::dx12:
    return "dx12";
  case BackendSelection::opengl:
    return "opengl";
  }
  return "unknown";
}

std::vector<BackendSelection> ordered_candidates(BackendSelection requested) {
  if (requested != BackendSelection::automatic) {
    return {requested};
  }
  std::vector<BackendSelection> candidates;
#if defined(_WIN32)
  candidates = {BackendSelection::dx12, BackendSelection::vulkan,
                BackendSelection::opengl};
#elif defined(__linux__)
  candidates = {BackendSelection::vulkan, BackendSelection::opengl};
#else
  candidates = {};
#endif
  return candidates;
}

struct ProbeResult {
  bool available = false;
  bool supported = false;
  std::string_view reason = "backend not available";
};

ProbeResult probe_backend(BackendSelection backend) noexcept {
  switch (backend) {
  case BackendSelection::vulkan:
#if defined(MANMENMI_HAS_VULKAN)
    return {true, true, "vulkan headers present"};
#else
    return {false, false, "vulkan headers unavailable"};
#endif
  case BackendSelection::dx12:
#if defined(MANMENMI_HAS_DX12)
    return {true, true, "dx12 headers present"};
#else
    return {false, false, "dx12 headers unavailable"};
#endif
  case BackendSelection::opengl:
#if defined(__has_include)
#if __has_include(                                                             \
    <GL/gl.h>) || __has_include(<OpenGL/gl.h>) || __has_include(<GLES3/gl3.h>)
    return {true, true, "opengl headers present"};
#else
    return {false, false, "opengl headers unavailable"};
#endif
#else
    return {false, false, "opengl headers unavailable"};
#endif
  case BackendSelection::automatic:
    return {false, true, "automatic selection resolves candidates"};
  }
  return {false, false, "unsupported backend selection"};
}

BackendCapabilities default_capabilities(BackendSelection backend) noexcept {
  BackendCapabilities capabilities{};
  capabilities.min_width = 1U;
  capabilities.min_height = 1U;
  capabilities.max_backbuffers = 3U;
  capabilities.adapter_name = "unavailable";
  capabilities.vendor_name = "unavailable";
  capabilities.presentable = false;
  capabilities.render_target_supported = false;
  capabilities.multi_buffering = false;
  capabilities.msaa_supported = false;
  capabilities.separate_graphics_queue = false;
  switch (backend) {
  case BackendSelection::vulkan:
  case BackendSelection::dx12:
  case BackendSelection::opengl:
  case BackendSelection::automatic:
    return capabilities;
  }
  return capabilities;
}

} // namespace detail

struct BufferState {
  virtual ~BufferState() = default;
  virtual Status upload(std::span<const std::uint8_t>) {
    return Status{Error{ErrorCode::unsupported_backend,
                        "buffer upload is not implemented for this backend"}};
  }
};

struct TextureState {
  virtual ~TextureState() = default;
};

struct PipelineState {
  virtual ~PipelineState() = default;
};

struct CommandBufferState;

struct DeviceState {
  virtual ~DeviceState() = default;
  virtual Status wait_idle() = 0;
  [[nodiscard]] virtual Result<std::unique_ptr<BufferState>>
  create_buffer(const BufferDescriptor &) {
    return Result<std::unique_ptr<BufferState>>{
        Error{ErrorCode::unsupported_backend,
              "buffer creation is not implemented for this backend"}};
  }
  [[nodiscard]] virtual Result<std::unique_ptr<TextureState>>
  create_texture(const TextureDescriptor &) {
    return Result<std::unique_ptr<TextureState>>{
        Error{ErrorCode::unsupported_backend,
              "texture creation is not implemented for this backend"}};
  }
  [[nodiscard]] virtual Result<std::unique_ptr<CommandBufferState>>
  create_command_buffer() {
    return Result<std::unique_ptr<CommandBufferState>>{
        Error{ErrorCode::unsupported_backend,
              "command buffer creation is not implemented for this backend"}};
  }
  [[nodiscard]] virtual Result<std::unique_ptr<PipelineState>>
  create_triangle_pipeline() {
    return Result<std::unique_ptr<PipelineState>>{Error{
        ErrorCode::unsupported_backend,
        "triangle pipeline creation is not implemented for this backend"}};
  }
};

class BufferImpl final : public Buffer {
public:
  BufferImpl(BufferDescriptor descriptor, std::unique_ptr<BufferState> state)
      : descriptor_(descriptor), state_(std::move(state)) {}

  [[nodiscard]] BufferState *state() noexcept { return state_.get(); }
  [[nodiscard]] const BufferState *state() const noexcept {
    return state_.get();
  }
  [[nodiscard]] const BufferDescriptor &descriptor() const noexcept {
    return descriptor_;
  }
  Status upload(std::span<const std::uint8_t> data) override {
    if (data.size() > descriptor_.size) {
      return Status{Error{ErrorCode::invalid_argument,
                          "buffer upload exceeds buffer size"}};
    }
    return state_->upload(data);
  }

private:
  BufferDescriptor descriptor_;
  std::unique_ptr<BufferState> state_;
};

class TextureImpl final : public Texture {
public:
  TextureImpl(TextureDescriptor descriptor, std::unique_ptr<TextureState> state)
      : descriptor_(descriptor), state_(std::move(state)) {}

  [[nodiscard]] TextureState *state() noexcept { return state_.get(); }
  [[nodiscard]] const TextureState *state() const noexcept {
    return state_.get();
  }
  [[nodiscard]] const TextureDescriptor &descriptor() const noexcept {
    return descriptor_;
  }

private:
  TextureDescriptor descriptor_;
  std::unique_ptr<TextureState> state_;
};

class PipelineImpl final : public Pipeline {
public:
  explicit PipelineImpl(std::unique_ptr<PipelineState> state)
      : state_(std::move(state)) {}

  [[nodiscard]] PipelineState *state() noexcept { return state_.get(); }
  [[nodiscard]] const PipelineState *state() const noexcept {
    return state_.get();
  }

private:
  std::unique_ptr<PipelineState> state_;
};

struct CommandBufferState {
  virtual ~CommandBufferState() = default;
  virtual Status begin() = 0;
  virtual Status copy_buffer(Buffer &, Buffer &, std::uint64_t, std::uint64_t,
                             std::uint64_t) = 0;
  virtual Status begin_render_pass(graphics::Swapchain &) {
    return Status{Error{ErrorCode::unsupported_backend,
                        "render pass is not implemented for this backend"}};
  }
  virtual Status begin_render_pass(Texture &) {
    return Status{
        Error{ErrorCode::unsupported_backend,
              "texture render pass is not implemented for this backend"}};
  }
  virtual Status set_pipeline(Pipeline &) {
    return Status{Error{ErrorCode::unsupported_backend,
                        "pipeline is not implemented for this backend"}};
  }
  virtual Status bind_vertex_buffer(Buffer &, std::uint64_t) {
    return Status{Error{ErrorCode::unsupported_backend,
                        "vertex buffers are not implemented for this backend"}};
  }
  virtual Status draw(std::uint32_t, std::uint32_t) {
    return Status{Error{ErrorCode::unsupported_backend,
                        "draw is not implemented for this backend"}};
  }
  virtual Status end_render_pass() {
    return Status{Error{ErrorCode::unsupported_backend,
                        "render pass is not implemented for this backend"}};
  }
  virtual Status close() = 0;
  virtual Status reset() = 0;
};

class CommandBufferImpl final : public CommandBuffer {
public:
  explicit CommandBufferImpl(std::unique_ptr<CommandBufferState> state)
      : state_(std::move(state)) {}

  Status begin() override { return state_->begin(); }
  Status copy_buffer(Buffer &source, Buffer &destination,
                     std::uint64_t source_offset,
                     std::uint64_t destination_offset,
                     std::uint64_t size) override {
    return state_->copy_buffer(source, destination, source_offset,
                               destination_offset, size);
  }
  Status begin_render_pass(graphics::Swapchain &swapchain) override {
    return state_->begin_render_pass(swapchain);
  }
  Status begin_render_pass(Texture &texture) override {
    return state_->begin_render_pass(texture);
  }
  Status set_pipeline(Pipeline &pipeline) override {
    return state_->set_pipeline(pipeline);
  }
  Status bind_vertex_buffer(Buffer &buffer, std::uint64_t offset) override {
    return state_->bind_vertex_buffer(buffer, offset);
  }
  Status draw(std::uint32_t vertex_count, std::uint32_t first_vertex) override {
    return state_->draw(vertex_count, first_vertex);
  }
  Status end_render_pass() override { return state_->end_render_pass(); }
  Status close() override { return state_->close(); }
  Status reset() override { return state_->reset(); }

  [[nodiscard]] CommandBufferState *state() noexcept { return state_.get(); }
  [[nodiscard]] const CommandBufferState *state() const noexcept {
    return state_.get();
  }

private:
  std::unique_ptr<CommandBufferState> state_;
};

class DeviceImpl final : public Device {
public:
  DeviceImpl(BackendSelection backend, BackendCapabilities capabilities,
             std::unique_ptr<DeviceState> state)
      : backend_(backend), capabilities_(capabilities),
        state_(std::move(state)) {}

  [[nodiscard]] BackendSelection backend() const noexcept override {
    return backend_;
  }

  [[nodiscard]] BackendCapabilities capabilities() const noexcept override {
    return capabilities_;
  }

  DeviceState *state() noexcept { return state_.get(); }
  const DeviceState *state() const noexcept { return state_.get(); }

  Status wait_idle() override {
    if (!state_) {
      return Status{Error{ErrorCode::not_initialized,
                          "graphics device state is missing"}};
    }

    return state_->wait_idle();
  }

  [[nodiscard]] Result<std::unique_ptr<Buffer>>
  create_buffer(const BufferDescriptor &descriptor) override {
    if (descriptor.size == 0U) {
      return Result<std::unique_ptr<Buffer>>{
          Error{ErrorCode::invalid_argument, "buffer size must be non-zero"}};
    }
    switch (descriptor.usage) {
    case BufferUsage::transfer_source:
    case BufferUsage::transfer_destination:
    case BufferUsage::vertex:
      break;
    default:
      return Result<std::unique_ptr<Buffer>>{
          Error{ErrorCode::invalid_argument, "buffer usage is invalid"}};
    }
    if (!state_) {
      return Result<std::unique_ptr<Buffer>>{Error{
          ErrorCode::not_initialized, "graphics device state is missing"}};
    }
    auto state_result = state_->create_buffer(descriptor);
    if (!state_result) {
      return Result<std::unique_ptr<Buffer>>{state_result.error()};
    }
    return Result<std::unique_ptr<Buffer>>{std::make_unique<BufferImpl>(
        descriptor, std::move(state_result).value())};
  }

  [[nodiscard]] Result<std::unique_ptr<Texture>>
  create_texture(const TextureDescriptor &descriptor) override {
    if (descriptor.width == 0U || descriptor.height == 0U) {
      return Result<std::unique_ptr<Texture>>{Error{
          ErrorCode::invalid_argument, "texture dimensions must be non-zero"}};
    }
    if (descriptor.format != TextureFormat::rgba8_unorm ||
        descriptor.usage != TextureUsage::render_target ||
        descriptor.layout != TextureLayout::opaque_render_target) {
      return Result<std::unique_ptr<Texture>>{
          Error{ErrorCode::invalid_argument,
                "texture format, usage, or layout is unsupported"}};
    }
    if (!state_) {
      return Result<std::unique_ptr<Texture>>{Error{
          ErrorCode::not_initialized, "graphics device state is missing"}};
    }
    auto state_result = state_->create_texture(descriptor);
    if (!state_result) {
      return Result<std::unique_ptr<Texture>>{state_result.error()};
    }
    return Result<std::unique_ptr<Texture>>{std::make_unique<TextureImpl>(
        descriptor, std::move(state_result).value())};
  }

  [[nodiscard]] Result<std::unique_ptr<CommandBuffer>>
  create_command_buffer() override {
    if (!state_) {
      return Result<std::unique_ptr<CommandBuffer>>{Error{
          ErrorCode::not_initialized, "graphics device state is missing"}};
    }
    auto state_result = state_->create_command_buffer();
    if (!state_result) {
      return Result<std::unique_ptr<CommandBuffer>>{state_result.error()};
    }
    return Result<std::unique_ptr<CommandBuffer>>{
        std::make_unique<CommandBufferImpl>(std::move(state_result).value())};
  }

  [[nodiscard]] Result<std::unique_ptr<Pipeline>>
  create_triangle_pipeline() override {
    if (!state_) {
      return Result<std::unique_ptr<Pipeline>>{Error{
          ErrorCode::not_initialized, "graphics device state is missing"}};
    }
    auto state_result = state_->create_triangle_pipeline();
    if (!state_result) {
      return Result<std::unique_ptr<Pipeline>>{state_result.error()};
    }
    return Result<std::unique_ptr<Pipeline>>{
        std::make_unique<PipelineImpl>(std::move(state_result).value())};
  }

private:
  BackendSelection backend_;
  BackendCapabilities capabilities_;
  std::unique_ptr<DeviceState> state_;
};

class QueueImpl final : public Queue {
public:
  QueueImpl() = default;

#if defined(MANMENMI_HAS_VULKAN)
  QueueImpl(BackendSelection backend, VkQueue vk_queue, VkDevice vk_device,
            VkSwapchainKHR vk_swapchain, std::uint32_t queue_family_index,
            VkSemaphore image_available, VkSemaphore render_finished)
      : backend_(backend), vk_queue_(vk_queue), vk_device_(vk_device),
        vk_swapchain_(vk_swapchain), queue_family_index_(queue_family_index),
        image_available_semaphore_(image_available),
        render_finished_semaphore_(render_finished) {
#if defined(MANMENMI_HAS_DX12)
    dx12_queue_ = nullptr;
    dx12_swapchain_ = nullptr;
#endif
    VkCommandPoolCreateInfo pool_info{
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, 0,
        queue_family_index_};
    if (vkCreateCommandPool(vk_device_, &pool_info, nullptr, &command_pool_) !=
        VK_SUCCESS) {
      command_pool_ = VK_NULL_HANDLE;
    }
    if (command_pool_ != VK_NULL_HANDLE) {
      VkCommandBufferAllocateInfo alloc_info{
          VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr,
          command_pool_, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1U};
      vkAllocateCommandBuffers(vk_device_, &alloc_info, &command_buffer_);
    }
  }

  [[nodiscard]] Result<std::unique_ptr<CommandBuffer>>
  create_command_buffer() override {
    if (!state_) {
      return Result<std::unique_ptr<CommandBuffer>>{Error{
          ErrorCode::not_initialized, "graphics device state is missing"}};
    }
    auto state_result = state_->create_command_buffer();
    if (!state_result) {
      return Result<std::unique_ptr<CommandBuffer>>{state_result.error()};
    }
    return Result<std::unique_ptr<CommandBuffer>>{
        std::make_unique<CommandBufferImpl>(std::move(state_result).value())};
  }
#endif

#if defined(MANMENMI_HAS_DX12)
  QueueImpl(BackendSelection backend, ID3D12Device *dx12_device,
            ID3D12CommandQueue *dx12_queue, IDXGISwapChain *dx12_swapchain)
      : backend_(backend), dx12_device_(dx12_device), dx12_queue_(dx12_queue),
        dx12_swapchain_(dx12_swapchain) {
#if defined(MANMENMI_HAS_VULKAN)
    vk_queue_ = VK_NULL_HANDLE;
    vk_device_ = VK_NULL_HANDLE;
    vk_swapchain_ = VK_NULL_HANDLE;
#endif
    if (dx12_device_ != nullptr) {
      dx12_device_->AddRef();
    }
    if (dx12_queue_ != nullptr) {
      dx12_queue_->AddRef();
    }
    if (dx12_swapchain_ != nullptr) {
      dx12_swapchain_->AddRef();
    }

    if (dx12_device_ != nullptr) {
      const HRESULT alloc_hr = dx12_device_->CreateCommandAllocator(
          D3D12_COMMAND_LIST_TYPE_DIRECT,
          IID_PPV_ARGS(&dx12_command_allocator_));
      if (FAILED(alloc_hr) || dx12_command_allocator_ == nullptr) {
        dx12_command_allocator_ = nullptr;
      }

      const HRESULT list_hr = dx12_device_->CreateCommandList(
          0U, D3D12_COMMAND_LIST_TYPE_DIRECT, dx12_command_allocator_, nullptr,
          IID_PPV_ARGS(&dx12_command_list_));
      if (FAILED(list_hr) || dx12_command_list_ == nullptr) {
        dx12_command_list_ = nullptr;
      } else if (FAILED(dx12_command_list_->Close())) {
        dx12_command_list_->Release();
        dx12_command_list_ = nullptr;
      }

      if (dx12_device_->CreateFence(0ULL, D3D12_FENCE_FLAG_NONE,
                                    IID_PPV_ARGS(&dx12_fence_)) != S_OK) {
        dx12_fence_ = nullptr;
      }
      dx12_fence_event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    }
  }
#endif

  ~QueueImpl() override {
#if defined(MANMENMI_HAS_VULKAN)
    if (command_buffer_ != VK_NULL_HANDLE && command_pool_ != VK_NULL_HANDLE) {
      vkFreeCommandBuffers(vk_device_, command_pool_, 1U, &command_buffer_);
    }
    if (command_pool_ != VK_NULL_HANDLE) {
      vkDestroyCommandPool(vk_device_, command_pool_, nullptr);
    }
#endif
#if defined(MANMENMI_HAS_DX12)
    if (dx12_command_list_ != nullptr) {
      dx12_command_list_->Release();
    }
    if (dx12_command_allocator_ != nullptr) {
      dx12_command_allocator_->Release();
    }
    if (dx12_fence_event_ != nullptr) {
      CloseHandle(dx12_fence_event_);
    }
    if (dx12_fence_ != nullptr) {
      dx12_fence_->Release();
    }
    if (dx12_swapchain_ != nullptr) {
      dx12_swapchain_->Release();
    }
    if (dx12_queue_ != nullptr) {
      dx12_queue_->Release();
    }
    if (dx12_device_ != nullptr) {
      dx12_device_->Release();
    }
#endif
  }

#if defined(MANMENMI_HAS_VULKAN)
  [[nodiscard]] VkQueue vk_queue() const noexcept { return vk_queue_; }
  [[nodiscard]] VkDevice vk_device() const noexcept { return vk_device_; }
  [[nodiscard]] VkSwapchainKHR vk_swapchain() const noexcept {
    return vk_swapchain_;
  }
  [[nodiscard]] VkSemaphore image_available_semaphore() const noexcept {
    return image_available_semaphore_;
  }
  [[nodiscard]] VkSemaphore render_finished_semaphore() const noexcept {
    return render_finished_semaphore_;
  }
#endif
#if defined(MANMENMI_HAS_DX12)
  [[nodiscard]] ID3D12CommandQueue *dx12_queue_handle() const noexcept {
    return dx12_queue_;
  }
  [[nodiscard]] IDXGISwapChain *dx12_swapchain_handle() const noexcept {
    return dx12_swapchain_;
  }
  [[nodiscard]] ID3D12Fence *dx12_fence_handle() const noexcept {
    return dx12_fence_;
  }
  [[nodiscard]] ID3D12GraphicsCommandList *
  dx12_command_list_handle() const noexcept {
    return dx12_command_list_;
  }
#endif

  Status submit() override {
#if defined(MANMENMI_HAS_VULKAN)
    if (backend_ == BackendSelection::vulkan) {
      if (vk_queue_ == VK_NULL_HANDLE || vk_device_ == VK_NULL_HANDLE) {
        return Status{
            Error{ErrorCode::not_initialized, "Vulkan queue is missing"}};
      }
      if (command_pool_ == VK_NULL_HANDLE ||
          command_buffer_ == VK_NULL_HANDLE) {
        return Status{Error{ErrorCode::not_initialized,
                            "Vulkan command buffer is missing"}};
      }

      VkCommandBufferBeginInfo begin_info{
          VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, 0, nullptr};
      if (vkBeginCommandBuffer(command_buffer_, &begin_info) != VK_SUCCESS) {
        return Status{Error{ErrorCode::device_lost,
                            "Vulkan command buffer begin failed"}};
      }
      if (vkEndCommandBuffer(command_buffer_) != VK_SUCCESS) {
        return Status{
            Error{ErrorCode::device_lost, "Vulkan command buffer end failed"}};
      }

      const VkPipelineStageFlags wait_stage =
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO,
                               nullptr,
                               1U,
                               &image_available_semaphore_,
                               &wait_stage,
                               1U,
                               &command_buffer_,
                               1U,
                               &render_finished_semaphore_};
      const VkResult result =
          vkQueueSubmit(vk_queue_, 1U, &submit_info, VK_NULL_HANDLE);
      if (result == VK_SUCCESS) {
        return success();
      }
      return Status{
          Error{ErrorCode::device_lost, "Vulkan queue submission failed"}};
    }
#endif
#if defined(MANMENMI_HAS_DX12)
    if (backend_ == BackendSelection::dx12) {
      if (dx12_queue_ == nullptr || dx12_swapchain_ == nullptr ||
          dx12_device_ == nullptr || dx12_command_allocator_ == nullptr ||
          dx12_command_list_ == nullptr || dx12_fence_ == nullptr ||
          dx12_fence_event_ == nullptr) {
        return Status{Error{ErrorCode::not_initialized,
                            "DX12 queue resources are missing"}};
      }

      HRESULT hr = dx12_command_allocator_->Reset();
      if (FAILED(hr)) {
        return Status{Error{ErrorCode::device_lost,
                            "DX12 command allocator reset failed"}};
      }
      hr = dx12_command_list_->Reset(dx12_command_allocator_, nullptr);
      if (FAILED(hr)) {
        return Status{
            Error{ErrorCode::device_lost, "DX12 command list reset failed"}};
      }
      hr = dx12_command_list_->Close();
      if (FAILED(hr)) {
        return Status{
            Error{ErrorCode::device_lost, "DX12 command list close failed"}};
      }

      ID3D12CommandList *command_lists[] = {dx12_command_list_};
      dx12_queue_->ExecuteCommandLists(1U, command_lists);

      const UINT64 fence_value = ++dx12_fence_value_;
      hr = dx12_queue_->Signal(dx12_fence_, fence_value);
      if (FAILED(hr)) {
        return Status{
            Error{ErrorCode::device_lost, "DX12 queue signal failed"}};
      }
      if (dx12_fence_->GetCompletedValue() < fence_value) {
        hr = dx12_fence_->SetEventOnCompletion(fence_value, dx12_fence_event_);
        if (FAILED(hr)) {
          return Status{Error{ErrorCode::device_lost,
                              "DX12 fence completion event failed"}};
        }
        if (WaitForSingleObject(dx12_fence_event_, INFINITE) != WAIT_OBJECT_0) {
          return Status{
              Error{ErrorCode::device_lost, "DX12 fence wait was interrupted"}};
        }
      }
      return success();
    }
#endif
    return Status{
        Error{ErrorCode::unimplemented,
              "submit() is not implemented for the requested backend"}};
  }

  Status submit(CommandBuffer &command_buffer) override;

  Status present() override {
#if defined(MANMENMI_HAS_VULKAN)
    if (backend_ == BackendSelection::vulkan) {
      if (vk_queue_ == VK_NULL_HANDLE || vk_swapchain_ == VK_NULL_HANDLE) {
        return Status{Error{ErrorCode::not_initialized,
                            "Vulkan queue/swapchain is missing"}};
      }
      const VkPresentInfoKHR info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                     nullptr,
                                     1,
                                     &render_finished_semaphore_,
                                     1,
                                     &vk_swapchain_,
                                     &image_index_};
      const VkResult result = vkQueuePresentKHR(vk_queue_, &info);
      if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
        return success();
      }
      if (result == VK_ERROR_OUT_OF_DATE_KHR ||
          result == VK_ERROR_SURFACE_LOST_KHR) {
        return Status{Error{ErrorCode::resize_failed,
                            "Vulkan swapchain must be recreated"}};
      }
      if (result == VK_ERROR_DEVICE_LOST) {
        return Status{Error{ErrorCode::device_lost,
                            "Vulkan device was lost during present"}};
      }
      return Status{Error{ErrorCode::present_failed, "Vulkan present failed"}};
    }
#endif
#if defined(MANMENMI_HAS_DX12)
    if (backend_ == BackendSelection::dx12) {
      if (dx12_swapchain_ == nullptr) {
        return Status{
            Error{ErrorCode::not_initialized, "DX12 swapchain is missing"}};
      }
      const HRESULT hr = dx12_swapchain_->Present(1, 0);
      if (SUCCEEDED(hr)) {
        return success();
      }
      if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        return Status{Error{ErrorCode::device_lost,
                            "DX12 device was lost during present"}};
      }
      return Status{Error{ErrorCode::present_failed, "DX12 present failed"}};
    }
#endif

    return Status{
        Error{ErrorCode::unimplemented,
              "present() is not implemented for the requested backend"}};
  }

  Status wait_idle() override {
#if defined(MANMENMI_HAS_VULKAN)
    if (backend_ == BackendSelection::vulkan) {
      if (vk_device_ == VK_NULL_HANDLE) {
        return Status{
            Error{ErrorCode::not_initialized, "Vulkan device is missing"}};
      }
      const VkResult result = vkDeviceWaitIdle(vk_device_);
      if (result == VK_SUCCESS) {
        return success();
      }
      return Status{
          Error{ErrorCode::device_lost, "Vulkan device wait idle failed"}};
    }
#endif
#if defined(MANMENMI_HAS_DX12)
    if (backend_ == BackendSelection::dx12) {
      if (dx12_queue_ == nullptr || dx12_fence_ == nullptr ||
          dx12_fence_event_ == nullptr) {
        return Status{Error{ErrorCode::not_initialized,
                            "DX12 synchronization resources are missing"}};
      }
      const UINT64 fence_value = ++dx12_fence_value_;
      HRESULT hr = dx12_queue_->Signal(dx12_fence_, fence_value);
      if (FAILED(hr)) {
        return Status{Error{ErrorCode::device_lost,
                            "DX12 queue signal failed during wait_idle"}};
      }
      if (dx12_fence_->GetCompletedValue() < fence_value) {
        hr = dx12_fence_->SetEventOnCompletion(fence_value, dx12_fence_event_);
        if (FAILED(hr)) {
          return Status{
              Error{ErrorCode::device_lost,
                    "DX12 fence completion event failed during wait_idle"}};
        }
        if (WaitForSingleObject(dx12_fence_event_, INFINITE) != WAIT_OBJECT_0) {
          return Status{
              Error{ErrorCode::device_lost,
                    "DX12 fence wait was interrupted during wait_idle"}};
        }
      }
      return success();
    }
#endif
    return Status{
        Error{ErrorCode::unimplemented,
              "wait_idle() is not implemented for the requested backend"}};
  }

  Status reset() override { return success(); }

  void set_image_index(std::uint32_t index) noexcept { image_index_ = index; }
#if defined(MANMENMI_HAS_VULKAN)
  void set_semaphores(VkSemaphore image_available,
                      VkSemaphore render_finished) noexcept {
    image_available_semaphore_ = image_available;
    render_finished_semaphore_ = render_finished;
  }
#endif

private:
  BackendSelection backend_ = BackendSelection::automatic;
#if defined(MANMENMI_HAS_VULKAN)
  VkQueue vk_queue_ = VK_NULL_HANDLE;
  VkDevice vk_device_ = VK_NULL_HANDLE;
  VkSwapchainKHR vk_swapchain_ = VK_NULL_HANDLE;
  std::uint32_t queue_family_index_ = 0U;
  VkSemaphore image_available_semaphore_ = VK_NULL_HANDLE;
  VkSemaphore render_finished_semaphore_ = VK_NULL_HANDLE;
  VkCommandPool command_pool_ = VK_NULL_HANDLE;
  VkCommandBuffer command_buffer_ = VK_NULL_HANDLE;
#endif
  std::uint32_t image_index_ = 0U;
#if defined(MANMENMI_HAS_DX12)
  ID3D12Device *dx12_device_ = nullptr;
  ID3D12CommandQueue *dx12_queue_ = nullptr;
  IDXGISwapChain *dx12_swapchain_ = nullptr;
  ID3D12CommandAllocator *dx12_command_allocator_ = nullptr;
  ID3D12GraphicsCommandList *dx12_command_list_ = nullptr;
  ID3D12Fence *dx12_fence_ = nullptr;
  HANDLE dx12_fence_event_ = nullptr;
  UINT64 dx12_fence_value_ = 0ULL;
#endif
};

#if defined(MANMENMI_HAS_VULKAN)
class VulkanDeviceState final : public DeviceState {
public:
  VulkanDeviceState(VkInstance instance, VkPhysicalDevice physical_device,
                    VkDevice device, VkQueue queue,
                    std::uint32_t queue_family_index)
      : instance_(instance), physical_device_(physical_device), device_(device),
        queue_(queue), queue_family_index_(queue_family_index) {}

  ~VulkanDeviceState() override {
    if (device_ != VK_NULL_HANDLE) {
      vkDestroyDevice(device_, nullptr);
    }
    if (instance_ != VK_NULL_HANDLE) {
      vkDestroyInstance(instance_, nullptr);
    }
  }

  [[nodiscard]] VkInstance instance() const noexcept { return instance_; }
  [[nodiscard]] VkPhysicalDevice physical_device() const noexcept {
    return physical_device_;
  }
  [[nodiscard]] VkDevice device() const noexcept { return device_; }
  [[nodiscard]] VkQueue queue() const noexcept { return queue_; }
  [[nodiscard]] std::uint32_t queue_family_index() const noexcept {
    return queue_family_index_;
  }

  Status wait_idle() override {
    if (device_ == VK_NULL_HANDLE) {
      return Status{Error{ErrorCode::not_initialized,
                          "Vulkan logical device is not initialized"}};
    }
    const VkResult result = vkDeviceWaitIdle(device_);
    if (result != VK_SUCCESS) {
      return Status{
          Error{ErrorCode::device_lost,
                std::string{"vkDeviceWaitIdle failed with code "}.append(
                    std::to_string(static_cast<int>(result)))}};
    }
    return success();
  }

  static Result<std::unique_ptr<DeviceState>> create(Logger &logger) {
    const auto app_info = VkApplicationInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                            nullptr,
                                            "MANMENMI",
                                            VK_MAKE_VERSION(0, 1, 0),
                                            "MANMENMI",
                                            VK_MAKE_VERSION(0, 1, 0),
                                            VK_API_VERSION_1_0};
    const auto create_info =
        VkInstanceCreateInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                             nullptr,
                             0,
                             &app_info,
                             0,
                             nullptr,
                             0,
                             nullptr};
    VkInstance instance = VK_NULL_HANDLE;
    const VkResult instance_result =
        vkCreateInstance(&create_info, nullptr, &instance);
    if (instance_result != VK_SUCCESS) {
      logger.write(LogLevel::warning, LogCategory::graphics,
                   std::string{"Vulkan instance creation failed: "}.append(
                       std::to_string(static_cast<int>(instance_result))));
      return Result<std::unique_ptr<DeviceState>>{
          Error{ErrorCode::device_creation_failed,
                "Vulkan instance creation failed"}};
    }

    std::uint32_t count = 0;
    VkResult enumerate_result =
        vkEnumeratePhysicalDevices(instance, &count, nullptr);
    if (enumerate_result != VK_SUCCESS || count == 0U) {
      vkDestroyInstance(instance, nullptr);
      logger.write(
          LogLevel::warning, LogCategory::graphics,
          "Vulkan physical device enumeration failed or returned no device");
      return Result<std::unique_ptr<DeviceState>>{Error{
          ErrorCode::unavailable, "Vulkan physical devices are unavailable"}};
    }

    std::vector<VkPhysicalDevice> physical_devices(count);
    enumerate_result =
        vkEnumeratePhysicalDevices(instance, &count, physical_devices.data());
    if (enumerate_result != VK_SUCCESS || physical_devices.empty()) {
      vkDestroyInstance(instance, nullptr);
      logger.write(
          LogLevel::warning, LogCategory::graphics,
          "Vulkan physical device enumeration returned no usable device");
      return Result<std::unique_ptr<DeviceState>>{Error{
          ErrorCode::unavailable, "Vulkan physical devices are unavailable"}};
    }

    VkPhysicalDevice physical_device = physical_devices.front();
    std::uint32_t queue_family_index = UINT32_MAX;
    std::uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
                                             &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device, &queue_family_count, queue_families.data());
    for (std::uint32_t index = 0; index < queue_family_count; ++index) {
      if (queue_families[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        queue_family_index = index;
        break;
      }
    }
    if (queue_family_index == UINT32_MAX) {
      vkDestroyInstance(instance, nullptr);
      logger.write(LogLevel::warning, LogCategory::graphics,
                   "Vulkan physical device has no graphics queue family");
      return Result<std::unique_ptr<DeviceState>>{
          Error{ErrorCode::unsupported_backend,
                "Vulkan physical device has no graphics queue"}};
    }

    const float queue_priority = 1.0f;
    const VkDeviceQueueCreateInfo queue_create_info{
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        nullptr,
        0,
        queue_family_index,
        1,
        &queue_priority};
    const VkDeviceCreateInfo device_create_info{
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        nullptr,
        0,
        1,
        &queue_create_info,
        0,
        nullptr,
        0,
        nullptr,
        nullptr};
    VkDevice device = VK_NULL_HANDLE;
    const VkResult device_result =
        vkCreateDevice(physical_device, &device_create_info, nullptr, &device);
    if (device_result != VK_SUCCESS) {
      vkDestroyInstance(instance, nullptr);
      logger.write(LogLevel::warning, LogCategory::graphics,
                   std::string{"Vulkan device creation failed: "}.append(
                       std::to_string(static_cast<int>(device_result))));
      return Result<std::unique_ptr<DeviceState>>{Error{
          ErrorCode::device_creation_failed, "Vulkan device creation failed"}};
    }

    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(device, queue_family_index, 0, &queue);
    if (queue == VK_NULL_HANDLE) {
      vkDestroyDevice(device, nullptr);
      vkDestroyInstance(instance, nullptr);
      logger.write(LogLevel::warning, LogCategory::graphics,
                   "Vulkan queue acquisition failed");
      return Result<std::unique_ptr<DeviceState>>{
          Error{ErrorCode::device_creation_failed,
                "Vulkan queue acquisition failed"}};
    }

    logger.write(LogLevel::info, LogCategory::graphics,
                 "Vulkan device initialized successfully");
    return Result<std::unique_ptr<DeviceState>>{
        std::make_unique<VulkanDeviceState>(instance, physical_device, device,
                                            queue, queue_family_index)};
  }

private:
  VkInstance instance_;
  VkPhysicalDevice physical_device_;
  VkDevice device_;
  VkQueue queue_;
  std::uint32_t queue_family_index_;
};
#endif

#if defined(MANMENMI_HAS_DX12)
class Dx12BufferState final : public BufferState {
public:
  explicit Dx12BufferState(ID3D12Resource *resource) : resource_(resource) {}

  ~Dx12BufferState() override {
    if (resource_ != nullptr) {
      resource_->Release();
    }
  }

  [[nodiscard]] ID3D12Resource *resource() const noexcept { return resource_; }

  Status upload(std::span<const std::uint8_t> data) override {
    if (resource_ == nullptr || data.empty()) {
      return Status{Error{ErrorCode::invalid_argument,
                          "DX12 buffer upload data is empty"}};
    }
    void *mapped = nullptr;
    if (FAILED(resource_->Map(0U, nullptr, &mapped)) || mapped == nullptr) {
      return Status{
          Error{ErrorCode::device_lost, "DX12 buffer upload mapping failed"}};
    }
    std::memcpy(mapped, data.data(), data.size());
    const D3D12_RANGE written_range{0U, data.size()};
    resource_->Unmap(0U, &written_range);
    return success();
  }

private:
  ID3D12Resource *resource_;
};

class Dx12TextureState final : public TextureState {
public:
  Dx12TextureState(ID3D12Resource *resource, ID3D12DescriptorHeap *rtv_heap,
                   D3D12_CPU_DESCRIPTOR_HANDLE rtv, std::uint32_t width,
                   std::uint32_t height)
      : resource_(resource), rtv_heap_(rtv_heap), rtv_(rtv), width_(width),
        height_(height) {}

  ~Dx12TextureState() override {
    if (rtv_heap_ != nullptr) {
      rtv_heap_->Release();
    }
    if (resource_ != nullptr) {
      resource_->Release();
    }
  }

  [[nodiscard]] ID3D12Resource *resource() const noexcept { return resource_; }
  [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE rtv() const noexcept {
    return rtv_;
  }
  [[nodiscard]] std::uint32_t width() const noexcept { return width_; }
  [[nodiscard]] std::uint32_t height() const noexcept { return height_; }

  static Result<std::unique_ptr<TextureState>>
  create(ID3D12Device *device, const TextureDescriptor &descriptor) {
    D3D12_HEAP_PROPERTIES heap_properties{};
    heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
    D3D12_RESOURCE_DESC resource_desc{};
    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resource_desc.Width = descriptor.width;
    resource_desc.Height = descriptor.height;
    resource_desc.DepthOrArraySize = 1U;
    resource_desc.MipLevels = 1U;
    resource_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    resource_desc.SampleDesc.Count = 1U;
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resource_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    const D3D12_CLEAR_VALUE clear_value{DXGI_FORMAT_R8G8B8A8_UNORM,
                                        {0.04F, 0.05F, 0.08F, 1.0F}};
    ID3D12Resource *resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
        D3D12_RESOURCE_STATE_RENDER_TARGET, &clear_value,
        IID_PPV_ARGS(&resource));
    if (FAILED(hr) || resource == nullptr) {
      return Result<std::unique_ptr<TextureState>>{Error{
          ErrorCode::out_of_memory, "DX12 texture resource creation failed"}};
    }
    D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
    heap_desc.NumDescriptors = 1U;
    heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    ID3D12DescriptorHeap *rtv_heap = nullptr;
    hr = device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&rtv_heap));
    if (FAILED(hr) || rtv_heap == nullptr) {
      resource->Release();
      return Result<std::unique_ptr<TextureState>>{
          Error{ErrorCode::device_creation_failed,
                "DX12 texture RTV heap creation failed"}};
    }
    const auto rtv = rtv_heap->GetCPUDescriptorHandleForHeapStart();
    device->CreateRenderTargetView(resource, nullptr, rtv);
    return Result<std::unique_ptr<TextureState>>{
        std::make_unique<Dx12TextureState>(
            resource, rtv_heap, rtv, descriptor.width, descriptor.height)};
  }

private:
  ID3D12Resource *resource_;
  ID3D12DescriptorHeap *rtv_heap_;
  D3D12_CPU_DESCRIPTOR_HANDLE rtv_;
  std::uint32_t width_;
  std::uint32_t height_;
};

struct Dx12RenderTargetBinding {
  ID3D12Resource *resource = nullptr;
  D3D12_CPU_DESCRIPTOR_HANDLE rtv{};
  std::uint32_t width = 0U;
  std::uint32_t height = 0U;
  bool texture_target = false;
};

[[nodiscard]] bool
resolve_dx12_render_target(graphics::Swapchain &swapchain,
                           Dx12RenderTargetBinding &binding) noexcept;
[[nodiscard]] bool
resolve_dx12_render_target(Texture &texture,
                           Dx12RenderTargetBinding &binding) noexcept;

class Dx12PipelineState final : public PipelineState {
public:
  Dx12PipelineState(ID3D12RootSignature *root_signature,
                    ID3D12PipelineState *pipeline_state)
      : root_signature_(root_signature), pipeline_state_(pipeline_state) {}

  ~Dx12PipelineState() override {
    if (pipeline_state_ != nullptr) {
      pipeline_state_->Release();
    }
    if (root_signature_ != nullptr) {
      root_signature_->Release();
    }
  }

  [[nodiscard]] ID3D12RootSignature *root_signature() const noexcept {
    return root_signature_;
  }
  [[nodiscard]] ID3D12PipelineState *pipeline_state() const noexcept {
    return pipeline_state_;
  }

  static Result<std::unique_ptr<PipelineState>> create(ID3D12Device *device) {
    if (device == nullptr) {
      return Result<std::unique_ptr<PipelineState>>{
          Error{ErrorCode::not_initialized, "DX12 device is missing"}};
    }

    const auto vertex_source_result =
        shader_ir::translate_to_hlsl(shader_ir::triangle_vertex_module());
    if (!vertex_source_result) {
      return Result<std::unique_ptr<PipelineState>>{
          vertex_source_result.error()};
    }
    const auto pixel_source_result =
        shader_ir::translate_to_hlsl(shader_ir::triangle_pixel_module());
    if (!pixel_source_result) {
      return Result<std::unique_ptr<PipelineState>>{
          pixel_source_result.error()};
    }
    const auto &vertex_shader_source = vertex_source_result.value().source;
    const auto &pixel_shader_source = pixel_source_result.value().source;

    ID3DBlob *vertex_blob = nullptr;
    ID3DBlob *pixel_blob = nullptr;
    ID3DBlob *error_blob = nullptr;
    const UINT compile_flags = D3DCOMPILE_ENABLE_STRICTNESS;
    HRESULT hr =
        D3DCompile(vertex_shader_source.data(), vertex_shader_source.size(),
                   "manmenmi_m3_vertex", nullptr, nullptr, "main", "vs_5_0",
                   compile_flags, 0U, &vertex_blob, &error_blob);
    if (error_blob != nullptr) {
      error_blob->Release();
      error_blob = nullptr;
    }
    if (FAILED(hr) || vertex_blob == nullptr) {
      return Result<std::unique_ptr<PipelineState>>{
          Error{ErrorCode::device_creation_failed,
                "DX12 triangle vertex shader compilation failed"}};
    }
    hr = D3DCompile(pixel_shader_source.data(), pixel_shader_source.size(),
                    "manmenmi_m3_pixel", nullptr, nullptr, "main", "ps_5_0",
                    compile_flags, 0U, &pixel_blob, &error_blob);
    if (error_blob != nullptr) {
      error_blob->Release();
      error_blob = nullptr;
    }
    if (FAILED(hr) || pixel_blob == nullptr) {
      vertex_blob->Release();
      return Result<std::unique_ptr<PipelineState>>{
          Error{ErrorCode::device_creation_failed,
                "DX12 triangle pixel shader compilation failed"}};
    }

    const D3D12_ROOT_SIGNATURE_DESC root_desc{
        0U, nullptr, 0U, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT};
    ID3DBlob *root_blob = nullptr;
    hr = D3D12SerializeRootSignature(&root_desc, D3D_ROOT_SIGNATURE_VERSION_1,
                                     &root_blob, &error_blob);
    if (error_blob != nullptr) {
      error_blob->Release();
      error_blob = nullptr;
    }
    if (FAILED(hr) || root_blob == nullptr) {
      pixel_blob->Release();
      vertex_blob->Release();
      return Result<std::unique_ptr<PipelineState>>{
          Error{ErrorCode::device_creation_failed,
                "DX12 triangle root signature serialization failed"}};
    }

    ID3D12RootSignature *root_signature = nullptr;
    hr = device->CreateRootSignature(0U, root_blob->GetBufferPointer(),
                                     root_blob->GetBufferSize(),
                                     IID_PPV_ARGS(&root_signature));
    root_blob->Release();
    if (FAILED(hr) || root_signature == nullptr) {
      pixel_blob->Release();
      vertex_blob->Release();
      return Result<std::unique_ptr<PipelineState>>{
          Error{ErrorCode::device_creation_failed,
                "DX12 triangle root signature creation failed"}};
    }

    D3D12_INPUT_ELEMENT_DESC input_elements[] = {
        {"POSITION", 0U, DXGI_FORMAT_R32G32_FLOAT, 0U, 0U,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0U},
        {"COLOR", 0U, DXGI_FORMAT_R32G32B32A32_FLOAT, 0U, 8U,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0U},
    };
    D3D12_BLEND_DESC blend_desc{};
    blend_desc.RenderTarget[0].RenderTargetWriteMask =
        D3D12_COLOR_WRITE_ENABLE_ALL;
    D3D12_RASTERIZER_DESC rasterizer_desc{};
    rasterizer_desc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizer_desc.CullMode = D3D12_CULL_MODE_NONE;
    rasterizer_desc.DepthClipEnable = TRUE;
    D3D12_DEPTH_STENCIL_DESC depth_desc{};
    depth_desc.DepthEnable = FALSE;
    depth_desc.StencilEnable = FALSE;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_desc{};
    pipeline_desc.pRootSignature = root_signature;
    pipeline_desc.VS = {vertex_blob->GetBufferPointer(),
                        vertex_blob->GetBufferSize()};
    pipeline_desc.PS = {pixel_blob->GetBufferPointer(),
                        pixel_blob->GetBufferSize()};
    pipeline_desc.BlendState = blend_desc;
    pipeline_desc.SampleMask = UINT_MAX;
    pipeline_desc.RasterizerState = rasterizer_desc;
    pipeline_desc.DepthStencilState = depth_desc;
    pipeline_desc.InputLayout = {input_elements, 2U};
    pipeline_desc.PrimitiveTopologyType =
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipeline_desc.NumRenderTargets = 1U;
    pipeline_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipeline_desc.SampleDesc.Count = 1U;
    ID3D12PipelineState *pipeline_state = nullptr;
    hr = device->CreateGraphicsPipelineState(&pipeline_desc,
                                             IID_PPV_ARGS(&pipeline_state));
    pixel_blob->Release();
    vertex_blob->Release();
    if (FAILED(hr) || pipeline_state == nullptr) {
      root_signature->Release();
      return Result<std::unique_ptr<PipelineState>>{
          Error{ErrorCode::device_creation_failed,
                "DX12 triangle pipeline creation failed"}};
    }
    return Result<std::unique_ptr<PipelineState>>{
        std::make_unique<Dx12PipelineState>(root_signature, pipeline_state)};
  }

private:
  ID3D12RootSignature *root_signature_;
  ID3D12PipelineState *pipeline_state_;
};

class Dx12CommandBufferState final : public CommandBufferState {
public:
  explicit Dx12CommandBufferState(ID3D12Device *device)
      : device_(device), allocator_(nullptr), list_(nullptr),
        state_(State::created) {
    if (device_ != nullptr) {
      device_->AddRef();
    }
    ensure_native_resources();
  }

  ~Dx12CommandBufferState() override {
    if (list_ != nullptr) {
      list_->Release();
    }
    if (allocator_ != nullptr) {
      allocator_->Release();
    }
    if (device_ != nullptr) {
      device_->Release();
    }
  }

  Status begin() override {
    if (state_ != State::created && state_ != State::resettable) {
      return Status{Error{ErrorCode::invalid_state,
                          "command buffer cannot begin in its current state"}};
    }
    if (!ensure_native_resources()) {
      return Status{
          Error{ErrorCode::not_initialized, "DX12 command buffer is missing"}};
    }
    HRESULT hr = allocator_->Reset();
    if (FAILED(hr)) {
      return Status{Error{ErrorCode::invalid_state,
                          "DX12 command allocator reset failed"}};
    }
    hr = list_->Reset(allocator_, nullptr);
    if (FAILED(hr)) {
      return Status{
          Error{ErrorCode::invalid_state, "DX12 command list reset failed"}};
    }
    state_ = State::recording;
    return success();
  }

  Status copy_buffer(Buffer &source, Buffer &destination,
                     std::uint64_t source_offset,
                     std::uint64_t destination_offset,
                     std::uint64_t size) override {
    if (state_ != State::recording) {
      return Status{Error{ErrorCode::invalid_state,
                          "CopyBuffer requires recording state"}};
    }
    auto *source_impl = dynamic_cast<BufferImpl *>(&source);
    auto *destination_impl = dynamic_cast<BufferImpl *>(&destination);
    if (source_impl == nullptr || destination_impl == nullptr) {
      return Status{Error{ErrorCode::invalid_argument,
                          "CopyBuffer received an invalid buffer"}};
    }
    const auto source_size = source_impl->descriptor().size;
    const auto destination_size = destination_impl->descriptor().size;
    if (source_impl->descriptor().usage != BufferUsage::transfer_source ||
        destination_impl->descriptor().usage !=
            BufferUsage::transfer_destination) {
      return Status{Error{ErrorCode::invalid_argument,
                          "CopyBuffer buffer usage is incompatible"}};
    }
    if (size == 0U || source_size < source_offset ||
        destination_size < destination_offset ||
        source_size - source_offset < size ||
        destination_size - destination_offset < size) {
      return Status{Error{ErrorCode::invalid_argument,
                          "CopyBuffer range is out of bounds"}};
    }
    const auto *source_state =
        dynamic_cast<const Dx12BufferState *>(source_impl->state());
    const auto *destination_state =
        dynamic_cast<const Dx12BufferState *>(destination_impl->state());
    if (source_state == nullptr || destination_state == nullptr ||
        source_state->resource() == nullptr ||
        destination_state->resource() == nullptr) {
      return Status{Error{ErrorCode::not_initialized,
                          "CopyBuffer native resources are missing"}};
    }
    if (source_state->resource() == destination_state->resource()) {
      return Status{Error{ErrorCode::invalid_argument,
                          "CopyBuffer source and destination overlap"}};
    }

    commands_.push_back(CopyBufferCommand{&source, &destination, source_offset,
                                          destination_offset, size});
    return success();
  }

  Status begin_render_pass(graphics::Swapchain &swapchain) override {
    if (state_ != State::recording || render_pass_active_) {
      return Status{Error{ErrorCode::invalid_state,
                          "render pass cannot begin in the current state"}};
    }
    Dx12RenderTargetBinding binding{};
    if (!resolve_dx12_render_target(swapchain, binding)) {
      return Status{Error{ErrorCode::invalid_argument,
                          "render pass target is not a DX12 swapchain"}};
    }
    commands_.push_back(BeginRenderPassCommand{&swapchain, nullptr});
    render_pass_active_ = true;
    return success();
  }

  Status begin_render_pass(Texture &texture) override {
    if (state_ != State::recording || render_pass_active_) {
      return Status{Error{ErrorCode::invalid_state,
                          "render pass cannot begin in the current state"}};
    }
    Dx12RenderTargetBinding binding{};
    if (!resolve_dx12_render_target(texture, binding)) {
      return Status{
          Error{ErrorCode::invalid_argument, "render pass texture is invalid"}};
    }
    commands_.push_back(BeginRenderPassCommand{nullptr, &texture});
    render_pass_active_ = true;
    return success();
  }

  Status set_pipeline(Pipeline &pipeline) override {
    if (state_ != State::recording || !render_pass_active_) {
      return Status{Error{ErrorCode::invalid_state,
                          "pipeline cannot be set outside a render pass"}};
    }
    auto *concrete = dynamic_cast<PipelineImpl *>(&pipeline);
    if (concrete == nullptr ||
        dynamic_cast<const Dx12PipelineState *>(concrete->state()) == nullptr) {
      return Status{Error{ErrorCode::invalid_argument,
                          "pipeline is not a DX12 triangle pipeline"}};
    }
    commands_.push_back(SetPipelineCommand{&pipeline});
    return success();
  }

  Status bind_vertex_buffer(Buffer &buffer, std::uint64_t offset) override {
    if (state_ != State::recording || !render_pass_active_) {
      return Status{
          Error{ErrorCode::invalid_state,
                "vertex buffer cannot be bound outside a render pass"}};
    }
    auto *concrete = dynamic_cast<BufferImpl *>(&buffer);
    if (concrete == nullptr ||
        concrete->descriptor().usage != BufferUsage::vertex ||
        offset >= concrete->descriptor().size) {
      return Status{Error{ErrorCode::invalid_argument,
                          "buffer is not a valid vertex buffer"}};
    }
    commands_.push_back(BindVertexBufferCommand{&buffer, offset});
    return success();
  }

  Status draw(std::uint32_t vertex_count, std::uint32_t first_vertex) override {
    if (state_ != State::recording || !render_pass_active_ ||
        vertex_count == 0U) {
      return Status{Error{ErrorCode::invalid_state,
                          "draw is invalid outside an active render pass"}};
    }
    commands_.push_back(DrawCommand{vertex_count, first_vertex});
    return success();
  }

  Status end_render_pass() override {
    if (state_ != State::recording || !render_pass_active_) {
      return Status{
          Error{ErrorCode::invalid_state, "render pass is not active"}};
    }
    commands_.push_back(EndRenderPassCommand{});
    render_pass_active_ = false;
    return success();
  }

  Status close() override {
    if (state_ != State::recording) {
      return Status{
          Error{ErrorCode::invalid_state, "command buffer is not recording"}};
    }
    if (render_pass_active_) {
      return Status{Error{ErrorCode::invalid_state,
                          "render pass must end before command buffer close"}};
    }
    if (!translate_commands()) {
      return Status{
          Error{ErrorCode::not_initialized, "DX12 command translation failed"}};
    }
    if (FAILED(list_->Close())) {
      return Status{
          Error{ErrorCode::device_lost, "DX12 command list close failed"}};
    }
    state_ = State::closed;
    return success();
  }

  Status reset() override {
    if (state_ != State::resettable) {
      return Status{
          Error{ErrorCode::invalid_state, "command buffer is not resettable"}};
    }
    commands_.clear();
    commands_translated_ = false;
    render_pass_active_ = false;
    state_ = State::created;
    return success();
  }

  [[nodiscard]] bool closed() const noexcept { return state_ == State::closed; }
  [[nodiscard]] bool in_flight() const noexcept {
    return state_ == State::in_flight;
  }
  [[nodiscard]] ID3D12CommandList *native_list() const noexcept {
    return list_;
  }
  void mark_in_flight() noexcept { state_ = State::in_flight; }
  void mark_gpu_complete() noexcept { state_ = State::resettable; }

private:
  struct CopyBufferCommand {
    Buffer *source;
    Buffer *destination;
    std::uint64_t source_offset;
    std::uint64_t destination_offset;
    std::uint64_t size;
  };

  struct BeginRenderPassCommand {
    graphics::Swapchain *swapchain;
    Texture *texture;
  };
  struct SetPipelineCommand {
    Pipeline *pipeline;
  };
  struct BindVertexBufferCommand {
    Buffer *buffer;
    std::uint64_t offset;
  };
  struct DrawCommand {
    std::uint32_t vertex_count;
    std::uint32_t first_vertex;
  };
  struct EndRenderPassCommand {};
  using Command = std::variant<CopyBufferCommand, BeginRenderPassCommand,
                               SetPipelineCommand, BindVertexBufferCommand,
                               DrawCommand, EndRenderPassCommand>;

  Status translate_commands() {
    if (commands_translated_) {
      return success();
    }
    Dx12RenderTargetBinding render_target{};
    bool render_pass_open = false;
    ID3D12PipelineState *pipeline_state = nullptr;
    ID3D12Resource *vertex_resource = nullptr;
    std::uint64_t vertex_offset = 0U;
    Status translation_status = success();
    for (const auto &command : commands_) {
      std::visit(
          [&](const auto &operation) {
            using Operation = std::decay_t<decltype(operation)>;
            if (!translation_status) {
              return;
            }
            if constexpr (std::is_same_v<Operation, CopyBufferCommand>) {
              auto *source_impl = dynamic_cast<BufferImpl *>(operation.source);
              auto *destination_impl =
                  dynamic_cast<BufferImpl *>(operation.destination);
              const auto *source_state =
                  source_impl == nullptr
                      ? nullptr
                      : dynamic_cast<const Dx12BufferState *>(
                            source_impl->state());
              const auto *destination_state =
                  destination_impl == nullptr
                      ? nullptr
                      : dynamic_cast<const Dx12BufferState *>(
                            destination_impl->state());
              if (source_state == nullptr || destination_state == nullptr ||
                  source_state->resource() == nullptr ||
                  destination_state->resource() == nullptr) {
                translation_status =
                    Status{Error{ErrorCode::not_initialized,
                                 "CopyBuffer command resources are missing"}};
                return;
              }
              D3D12_RESOURCE_BARRIER source_barrier{};
              source_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
              source_barrier.Transition.pResource = source_state->resource();
              source_barrier.Transition.StateBefore =
                  D3D12_RESOURCE_STATE_GENERIC_READ;
              source_barrier.Transition.StateAfter =
                  D3D12_RESOURCE_STATE_COPY_SOURCE;
              source_barrier.Transition.Subresource =
                  D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
              list_->ResourceBarrier(1U, &source_barrier);
              list_->CopyBufferRegion(destination_state->resource(),
                                      operation.destination_offset,
                                      source_state->resource(),
                                      operation.source_offset, operation.size);
              source_barrier.Transition.StateBefore =
                  D3D12_RESOURCE_STATE_COPY_SOURCE;
              source_barrier.Transition.StateAfter =
                  D3D12_RESOURCE_STATE_GENERIC_READ;
              list_->ResourceBarrier(1U, &source_barrier);
            } else if constexpr (std::is_same_v<Operation,
                                                BeginRenderPassCommand>) {
              const bool target_valid =
                  operation.swapchain != nullptr
                      ? resolve_dx12_render_target(*operation.swapchain,
                                                   render_target)
                      : operation.texture != nullptr &&
                            resolve_dx12_render_target(*operation.texture,
                                                       render_target);
              if (render_pass_open || !target_valid) {
                translation_status =
                    Status{Error{ErrorCode::invalid_state,
                                 "DX12 render pass target is invalid"}};
                return;
              }
              if (!render_target.texture_target) {
                D3D12_RESOURCE_BARRIER barrier{};
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Transition.pResource = render_target.resource;
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
                barrier.Transition.StateAfter =
                    D3D12_RESOURCE_STATE_RENDER_TARGET;
                barrier.Transition.Subresource =
                    D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                list_->ResourceBarrier(1U, &barrier);
              }
              list_->OMSetRenderTargets(1U, &render_target.rtv, FALSE, nullptr);
              const D3D12_VIEWPORT viewport{
                  0.0F,
                  0.0F,
                  static_cast<FLOAT>(render_target.width),
                  static_cast<FLOAT>(render_target.height),
                  0.0F,
                  1.0F};
              const D3D12_RECT scissor{0L, 0L,
                                       static_cast<LONG>(render_target.width),
                                       static_cast<LONG>(render_target.height)};
              list_->RSSetViewports(1U, &viewport);
              list_->RSSetScissorRects(1U, &scissor);
              const FLOAT clear_color[] = {0.04F, 0.05F, 0.08F, 1.0F};
              list_->ClearRenderTargetView(render_target.rtv, clear_color, 0U,
                                           nullptr);
              render_pass_open = true;
            } else if constexpr (std::is_same_v<Operation,
                                                SetPipelineCommand>) {
              auto *pipeline_impl =
                  dynamic_cast<PipelineImpl *>(operation.pipeline);
              auto *state = pipeline_impl == nullptr
                                ? nullptr
                                : dynamic_cast<Dx12PipelineState *>(
                                      pipeline_impl->state());
              if (!render_pass_open || state == nullptr) {
                translation_status = Status{
                    Error{ErrorCode::invalid_state,
                          "DX12 pipeline is invalid for the render pass"}};
                return;
              }
              list_->SetGraphicsRootSignature(state->root_signature());
              list_->SetPipelineState(state->pipeline_state());
              pipeline_state = state->pipeline_state();
            } else if constexpr (std::is_same_v<Operation,
                                                BindVertexBufferCommand>) {
              auto *buffer_impl = dynamic_cast<BufferImpl *>(operation.buffer);
              auto *state =
                  buffer_impl == nullptr
                      ? nullptr
                      : dynamic_cast<Dx12BufferState *>(buffer_impl->state());
              if (!render_pass_open || state == nullptr ||
                  state->resource() == nullptr ||
                  buffer_impl->descriptor().usage != BufferUsage::vertex) {
                translation_status = Status{
                    Error{ErrorCode::invalid_state,
                          "DX12 vertex buffer is invalid for the render pass"}};
                return;
              }
              vertex_resource = state->resource();
              vertex_offset = operation.offset;
              D3D12_VERTEX_BUFFER_VIEW view{};
              view.BufferLocation =
                  vertex_resource->GetGPUVirtualAddress() + vertex_offset;
              view.SizeInBytes = static_cast<UINT>(
                  buffer_impl->descriptor().size - vertex_offset);
              view.StrideInBytes = 24U;
              list_->IASetVertexBuffers(0U, 1U, &view);
            } else if constexpr (std::is_same_v<Operation, DrawCommand>) {
              if (!render_pass_open || pipeline_state == nullptr ||
                  vertex_resource == nullptr) {
                translation_status = Status{
                    Error{ErrorCode::invalid_state,
                          "DX12 draw is missing pipeline or vertex buffer"}};
                return;
              }
              list_->IASetPrimitiveTopology(
                  D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
              list_->DrawInstanced(operation.vertex_count, 1U,
                                   operation.first_vertex, 0U);
            } else if constexpr (std::is_same_v<Operation,
                                                EndRenderPassCommand>) {
              if (!render_pass_open || render_target.resource == nullptr) {
                translation_status = Status{Error{
                    ErrorCode::invalid_state, "DX12 render pass is not open"}};
                return;
              }
              if (!render_target.texture_target) {
                D3D12_RESOURCE_BARRIER barrier{};
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Transition.pResource = render_target.resource;
                barrier.Transition.StateBefore =
                    D3D12_RESOURCE_STATE_RENDER_TARGET;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
                barrier.Transition.Subresource =
                    D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                list_->ResourceBarrier(1U, &barrier);
              }
              render_pass_open = false;
            }
          },
          command);
      if (!translation_status) {
        return translation_status;
      }
    }
    if (render_pass_open) {
      return Status{
          Error{ErrorCode::invalid_state, "DX12 render pass was not closed"}};
    }
    commands_translated_ = true;
    return success();
  }

  bool ensure_native_resources() {
    if (device_ == nullptr) {
      return false;
    }
    if (allocator_ != nullptr && list_ != nullptr) {
      return true;
    }
    if (allocator_ == nullptr) {
      const HRESULT allocator_hr = device_->CreateCommandAllocator(
          D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator_));
      if (FAILED(allocator_hr) || allocator_ == nullptr) {
        allocator_ = nullptr;
        return false;
      }
    }
    if (list_ == nullptr) {
      const HRESULT list_hr =
          device_->CreateCommandList(0U, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                     allocator_, nullptr, IID_PPV_ARGS(&list_));
      if (FAILED(list_hr) || list_ == nullptr) {
        if (list_ != nullptr) {
          list_->Release();
        }
        list_ = nullptr;
        return false;
      }
      if (FAILED(list_->Close())) {
        list_->Release();
        list_ = nullptr;
        allocator_->Release();
        allocator_ = nullptr;
        return false;
      }
    }
    state_ = State::created;
    return true;
  }

  enum class State { created, recording, closed, in_flight, resettable };
  ID3D12Device *device_;
  ID3D12CommandAllocator *allocator_;
  ID3D12GraphicsCommandList *list_;
  State state_;
  std::vector<Command> commands_;
  bool commands_translated_ = false;
  bool render_pass_active_ = false;
};

class Dx12DeviceState final : public DeviceState {
public:
  explicit Dx12DeviceState(ID3D12Device *device, ID3D12CommandQueue *queue,
                           ID3D12Fence *fence, HANDLE event)
      : device_(device), queue_(queue), fence_(fence), fence_event_(event),
        fence_value_(0ULL) {}

  ~Dx12DeviceState() override {
    if (fence_event_ != nullptr) {
      CloseHandle(fence_event_);
    }
    if (fence_ != nullptr) {
      fence_->Release();
    }
    if (queue_ != nullptr) {
      queue_->Release();
    }
    if (device_ != nullptr) {
      device_->Release();
    }
  }

  [[nodiscard]] ID3D12Device *device() const noexcept { return device_; }
  [[nodiscard]] ID3D12CommandQueue *queue() const noexcept { return queue_; }

  [[nodiscard]] Result<std::unique_ptr<CommandBufferState>>
  create_command_buffer() override {
    auto state = std::make_unique<Dx12CommandBufferState>(device_);
    return Result<std::unique_ptr<CommandBufferState>>{std::move(state)};
  }

  [[nodiscard]] Result<std::unique_ptr<PipelineState>>
  create_triangle_pipeline() override {
    return Dx12PipelineState::create(device_);
  }

  [[nodiscard]] Result<std::unique_ptr<BufferState>>
  create_buffer(const BufferDescriptor &descriptor) override {
    D3D12_HEAP_PROPERTIES heap_properties{};
    heap_properties.Type = descriptor.usage == BufferUsage::transfer_source ||
                                   descriptor.usage == BufferUsage::vertex
                               ? D3D12_HEAP_TYPE_UPLOAD
                               : D3D12_HEAP_TYPE_READBACK;

    D3D12_RESOURCE_DESC resource_desc{};
    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resource_desc.Width = descriptor.size;
    resource_desc.Height = 1U;
    resource_desc.DepthOrArraySize = 1U;
    resource_desc.MipLevels = 1U;
    resource_desc.Format = DXGI_FORMAT_UNKNOWN;
    resource_desc.SampleDesc.Count = 1U;
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    const D3D12_RESOURCE_STATES initial_state =
        descriptor.usage == BufferUsage::transfer_source ||
                descriptor.usage == BufferUsage::vertex
            ? D3D12_RESOURCE_STATE_GENERIC_READ
            : D3D12_RESOURCE_STATE_COPY_DEST;

    ID3D12Resource *resource = nullptr;
    const HRESULT hr = device_->CreateCommittedResource(
        &heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, initial_state,
        nullptr, IID_PPV_ARGS(&resource));
    if (FAILED(hr) || resource == nullptr) {
      return Result<std::unique_ptr<BufferState>>{Error{
          ErrorCode::out_of_memory, "DX12 buffer resource creation failed"}};
    }
    return Result<std::unique_ptr<BufferState>>{
        std::make_unique<Dx12BufferState>(resource)};
  }

  [[nodiscard]] Result<std::unique_ptr<TextureState>>
  create_texture(const TextureDescriptor &descriptor) override {
    return Dx12TextureState::create(device_, descriptor);
  }

  Status wait_idle() override {
    if (queue_ == nullptr || fence_ == nullptr || fence_event_ == nullptr) {
      return Status{Error{ErrorCode::unavailable,
                          "DX12 queue/fence is not available for wait_idle"}};
    }
    const HRESULT signal = queue_->Signal(fence_, ++fence_value_);
    if (FAILED(signal)) {
      return Status{Error{ErrorCode::device_lost, "DX12 queue signal failed"}};
    }
    const HRESULT wait =
        fence_->SetEventOnCompletion(fence_value_, fence_event_);
    if (FAILED(wait)) {
      return Status{
          Error{ErrorCode::device_lost, "DX12 wait fence completion failed"}};
    }
    if (WaitForSingleObject(fence_event_, INFINITE) != WAIT_OBJECT_0) {
      return Status{
          Error{ErrorCode::device_lost, "DX12 wait for fence was interrupted"}};
    }
    return success();
  }

  static Result<std::unique_ptr<DeviceState>> create(Logger &logger) {
    ID3D12Device *device = nullptr;
    HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0,
                                   IID_PPV_ARGS(&device));
    if (FAILED(hr) || device == nullptr) {
      logger.write(LogLevel::warning, LogCategory::graphics,
                   "DX12 device creation failed");
      return Result<std::unique_ptr<DeviceState>>{Error{
          ErrorCode::device_creation_failed, "DX12 device creation failed"}};
    }

    D3D12_COMMAND_QUEUE_DESC queue_desc{};
    queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ID3D12CommandQueue *queue = nullptr;
    hr = device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&queue));
    if (FAILED(hr) || queue == nullptr) {
      device->Release();
      logger.write(LogLevel::warning, LogCategory::graphics,
                   "DX12 command queue creation failed");
      return Result<std::unique_ptr<DeviceState>>{
          Error{ErrorCode::device_creation_failed,
                "DX12 command queue creation failed"}};
    }

    ID3D12Fence *fence = nullptr;
    hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    if (FAILED(hr) || fence == nullptr) {
      queue->Release();
      device->Release();
      logger.write(LogLevel::warning, LogCategory::graphics,
                   "DX12 fence creation failed");
      return Result<std::unique_ptr<DeviceState>>{Error{
          ErrorCode::device_creation_failed, "DX12 fence creation failed"}};
    }

    HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (event == nullptr) {
      fence->Release();
      queue->Release();
      device->Release();
      logger.write(LogLevel::warning, LogCategory::graphics,
                   "DX12 wait event creation failed");
      return Result<std::unique_ptr<DeviceState>>{
          Error{ErrorCode::device_creation_failed,
                "DX12 wait event creation failed"}};
    }

    logger.write(LogLevel::info, LogCategory::graphics,
                 "DX12 device initialized successfully");
    return Result<std::unique_ptr<DeviceState>>{
        std::make_unique<Dx12DeviceState>(device, queue, fence, event)};
  }

private:
  ID3D12Device *device_;
  ID3D12CommandQueue *queue_;
  ID3D12Fence *fence_;
  HANDLE fence_event_;
  std::uint64_t fence_value_;
};
#endif

Status QueueImpl::submit(CommandBuffer &command_buffer) {
#if defined(MANMENMI_HAS_DX12)
  if (backend_ == BackendSelection::dx12) {
    auto *concrete = dynamic_cast<CommandBufferImpl *>(&command_buffer);
    if (concrete == nullptr) {
      return Status{Error{ErrorCode::invalid_argument,
                          "command buffer is not a MANMENMI command buffer"}};
    }
    auto *state = dynamic_cast<Dx12CommandBufferState *>(concrete->state());
    if (state == nullptr || !state->closed() || state->in_flight() ||
        state->native_list() == nullptr) {
      return Status{
          Error{ErrorCode::invalid_state,
                "DX12 command buffer is not closed or is already in flight"}};
    }
    if (dx12_queue_ == nullptr || dx12_fence_ == nullptr ||
        dx12_fence_event_ == nullptr) {
      return Status{Error{ErrorCode::not_initialized,
                          "DX12 synchronization resources are missing"}};
    }

    state->mark_in_flight();
    ID3D12CommandList *command_lists[] = {state->native_list()};
    dx12_queue_->ExecuteCommandLists(1U, command_lists);
    const UINT64 fence_value = ++dx12_fence_value_;
    HRESULT hr = dx12_queue_->Signal(dx12_fence_, fence_value);
    if (FAILED(hr)) {
      return Status{Error{ErrorCode::device_lost, "DX12 queue signal failed"}};
    }
    if (dx12_fence_->GetCompletedValue() < fence_value) {
      hr = dx12_fence_->SetEventOnCompletion(fence_value, dx12_fence_event_);
      if (FAILED(hr)) {
        return Status{Error{ErrorCode::device_lost,
                            "DX12 fence completion event failed"}};
      }
      if (WaitForSingleObject(dx12_fence_event_, INFINITE) != WAIT_OBJECT_0) {
        return Status{
            Error{ErrorCode::device_lost, "DX12 fence wait was interrupted"}};
      }
    }
    state->mark_gpu_complete();
    return success();
  }
#endif
  return Status{Error{
      ErrorCode::unsupported_backend,
      "command buffer submission is unavailable for the requested backend"}};
}

#if defined(MANMENMI_HAS_VULKAN)
class VulkanSwapchain final : public Swapchain {
public:
  VulkanSwapchain(VkInstance instance, VkDevice device,
                  VkPhysicalDevice physical_device, VkQueue queue,
                  VkSurfaceKHR surface, VkSwapchainKHR swapchain,
                  std::uint32_t image_count, std::uint32_t current_image_index,
                  VkFormat format, VkPresentModeKHR present_mode,
                  VkSurfaceCapabilitiesKHR capabilities)
      : instance_(instance), device_(device), physical_device_(physical_device),
        queue_(queue), surface_(surface), swapchain_(swapchain),
        image_count_(image_count), current_image_index_(current_image_index),
        format_(format), present_mode_(present_mode),
        capabilities_(capabilities), image_available_(VK_NULL_HANDLE),
        render_finished_(VK_NULL_HANDLE) {
    const VkSemaphoreCreateInfo sema_info{
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
    vkCreateSemaphore(device_, &sema_info, nullptr, &image_available_);
    vkCreateSemaphore(device_, &sema_info, nullptr, &render_finished_);
  }

  ~VulkanSwapchain() override {
    if (render_finished_ != VK_NULL_HANDLE) {
      vkDestroySemaphore(device_, render_finished_, nullptr);
    }
    if (image_available_ != VK_NULL_HANDLE) {
      vkDestroySemaphore(device_, image_available_, nullptr);
    }
    if (swapchain_ != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(device_, swapchain_, nullptr);
    }
    if (surface_ != VK_NULL_HANDLE) {
      vkDestroySurfaceKHR(instance_, surface_, nullptr);
    }
  }

  Status acquire_next_image() override {
    if (swapchain_ == VK_NULL_HANDLE || device_ == VK_NULL_HANDLE) {
      return Status{Error{ErrorCode::not_initialized,
                          "Vulkan swapchain is not initialized"}};
    }
    std::uint32_t index = 0U;
    const VkResult result =
        vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX, image_available_,
                              VK_NULL_HANDLE, &index);
    if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
      current_image_index_ = index;
      if (auto *concrete = dynamic_cast<QueueImpl *>(queue_impl_.get());
          concrete != nullptr) {
        concrete->set_image_index(index);
      }
      return success();
    }
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      return Status{
          Error{ErrorCode::resize_failed, "Vulkan swapchain is out of date"}};
    }
    if (result == VK_ERROR_DEVICE_LOST) {
      return Status{Error{ErrorCode::device_lost,
                          "Vulkan device was lost while acquiring"}};
    }
    return Status{Error{ErrorCode::swapchain_creation_failed,
                        "Vulkan image acquisition failed"}};
  }

  [[nodiscard]] std::uint32_t image_count() const noexcept override {
    return image_count_;
  }
  [[nodiscard]] std::uint32_t current_image_index() const noexcept override {
    return current_image_index_;
  }
  [[nodiscard]] bool native_runtime_ready_for_test() const noexcept {
    return swapchain_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE &&
           queue_ != VK_NULL_HANDLE && image_available_ != VK_NULL_HANDLE &&
           render_finished_ != VK_NULL_HANDLE && !images_.empty();
  }
  [[nodiscard]] Queue *queue_for_test() noexcept { return queue_impl_.get(); }
  [[nodiscard]] const Queue *queue_for_test() const noexcept {
    return queue_impl_.get();
  }

  Status resize(std::uint32_t width, std::uint32_t height) override {
    if (width == 0U || height == 0U) {
      return Status{Error{ErrorCode::resize_failed,
                          "Vulkan swapchain resize requires non-zero extent"}};
    }
    return recreate();
  }

  Status recreate() override {
    if (swapchain_ == VK_NULL_HANDLE || device_ == VK_NULL_HANDLE ||
        surface_ == VK_NULL_HANDLE) {
      return Status{Error{ErrorCode::not_initialized,
                          "Vulkan swapchain state is missing"}};
    }

    VkSurfaceCapabilitiesKHR capabilities{};
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            physical_device_, surface_, &capabilities) != VK_SUCCESS) {
      return Status{
          Error{ErrorCode::resize_failed,
                "Vulkan surface capabilities query failed during recreate"}};
    }

    VkExtent2D extent = capabilities.currentExtent;
    if (extent.width == UINT32_MAX || extent.height == UINT32_MAX) {
      extent.width = std::max<std::uint32_t>(1U, image_count_);
      extent.height = std::max<std::uint32_t>(1U, image_count_);
    }

    const VkSwapchainCreateInfoKHR create_info{
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        nullptr,
        0,
        surface_,
        image_count_,
        format_,
        VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        extent,
        1,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        capabilities.currentTransform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        present_mode_,
        VK_TRUE,
        swapchain_,
        VK_NULL_HANDLE,
    };

    VkSwapchainKHR replacement = VK_NULL_HANDLE;
    const VkResult result =
        vkCreateSwapchainKHR(device_, &create_info, nullptr, &replacement);
    if (result != VK_SUCCESS) {
      return Status{Error{ErrorCode::swapchain_creation_failed,
                          "Vulkan swapchain recreate failed"}};
    }

    const VkSwapchainKHR previous = swapchain_;
    swapchain_ = replacement;
    if (previous != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(device_, previous, nullptr);
    }
    return success();
  }

  static Result<std::unique_ptr<Swapchain>>
  create(Device &device, const SurfaceToken &surface_token, std::uint32_t width,
         std::uint32_t height, Logger &logger) {
    auto *concrete = dynamic_cast<DeviceImpl *>(&device);
    if (concrete == nullptr) {
      return Result<std::unique_ptr<Swapchain>>{
          Error{ErrorCode::not_initialized, "device is not a MANMENMI device"}};
    }
    auto *vk_state = dynamic_cast<VulkanDeviceState *>(concrete->state());
    if (vk_state == nullptr) {
      return Result<std::unique_ptr<Swapchain>>{
          Error{ErrorCode::unsupported_backend, "device is not Vulkan"}};
    }

    const auto surface_ref = resolve_surface_token(surface_token);
    if (!surface_ref.valid || surface_ref.window == nullptr) {
      return Result<std::unique_ptr<Swapchain>>{
          Error{ErrorCode::invalid_argument,
                "SurfaceToken does not resolve to a valid window"}};
    }

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (!SDL_Vulkan_CreateSurface(surface_ref.window, vk_state->instance(),
                                  &surface)) {
      logger.write(LogLevel::warning, LogCategory::graphics,
                   std::string{"SDL Vulkan surface creation failed: "}.append(
                       SDL_GetError()));
      return Result<std::unique_ptr<Swapchain>>{
          Error{ErrorCode::surface_creation_failed,
                "Vulkan surface creation failed"}};
    }

    VkPhysicalDevice physical_device = vk_state->physical_device();
    VkDevice device_handle = vk_state->device();

    std::uint32_t graphics_family = vk_state->queue_family_index();
    VkBool32 present_supported = VK_FALSE;
    const VkResult support_result = vkGetPhysicalDeviceSurfaceSupportKHR(
        physical_device, graphics_family, surface, &present_supported);
    if (support_result != VK_SUCCESS || present_supported != VK_TRUE) {
      vkDestroySurfaceKHR(vk_state->instance(), surface, nullptr);
      return Result<std::unique_ptr<Swapchain>>{
          Error{ErrorCode::unsupported_backend,
                "Vulkan queue family does not support presentation"}};
    }

    VkSurfaceCapabilitiesKHR capabilities{};
    const VkResult caps_result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physical_device, surface, &capabilities);
    if (caps_result != VK_SUCCESS) {
      vkDestroySurfaceKHR(vk_state->instance(), surface, nullptr);
      return Result<std::unique_ptr<Swapchain>>{
          Error{ErrorCode::surface_creation_failed,
                "Vulkan surface capabilities query failed"}};
    }

    std::uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
                                         &format_count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(format_count);
    if (format_count > 0U) {
      vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
                                           &format_count, formats.data());
    }
    std::uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                              &present_mode_count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    if (present_mode_count > 0U) {
      vkGetPhysicalDeviceSurfacePresentModesKHR(
          physical_device, surface, &present_mode_count, present_modes.data());
    }

    VkSurfaceFormatKHR chosen_format =
        formats.empty() ? VkSurfaceFormatKHR{VK_FORMAT_UNDEFINED,
                                             VK_COLORSPACE_SRGB_NONLINEAR_KHR}
                        : formats.front();
    for (const auto &candidate : formats) {
      if (candidate.format == VK_FORMAT_B8G8R8A8_SRGB ||
          candidate.format == VK_FORMAT_R8G8B8A8_SRGB) {
        chosen_format = candidate;
        break;
      }
    }

    VkPresentModeKHR chosen_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto &candidate : present_modes) {
      if (candidate == VK_PRESENT_MODE_MAILBOX_KHR) {
        chosen_present_mode = candidate;
        break;
      }
      if (candidate == VK_PRESENT_MODE_IMMEDIATE_KHR &&
          chosen_present_mode == VK_PRESENT_MODE_FIFO_KHR) {
        chosen_present_mode = candidate;
      }
    }

    VkExtent2D extent = capabilities.currentExtent;
    if (extent.width == UINT32_MAX || extent.height == UINT32_MAX) {
      extent.width = std::max<std::uint32_t>(
          1U,
          std::min<std::uint32_t>(width, capabilities.maxImageExtent.width));
      extent.height = std::max<std::uint32_t>(
          1U,
          std::min<std::uint32_t>(height, capabilities.maxImageExtent.height));
    }

    std::uint32_t min_image_count = capabilities.minImageCount + 1U;
    if (capabilities.maxImageCount > 0U) {
      min_image_count = std::min(min_image_count, capabilities.maxImageCount);
    }
    std::uint32_t image_count = std::max<std::uint32_t>(2U, min_image_count);

    const VkSwapchainCreateInfoKHR swapchain_info{
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        nullptr,
        0,
        surface,
        image_count,
        chosen_format.format,
        chosen_format.colorSpace,
        extent,
        1,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        capabilities.currentTransform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        chosen_present_mode,
        VK_TRUE,
        VK_NULL_HANDLE,
        VK_NULL_HANDLE};

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    const VkResult swapchain_result = vkCreateSwapchainKHR(
        device_handle, &swapchain_info, nullptr, &swapchain);
    if (swapchain_result != VK_SUCCESS) {
      vkDestroySurfaceKHR(vk_state->instance(), surface, nullptr);
      return Result<std::unique_ptr<Swapchain>>{
          Error{ErrorCode::swapchain_creation_failed,
                "Vulkan swapchain creation failed"}};
    }

    std::uint32_t acquired_index = 0U;
    const VkResult acquire_result =
        vkAcquireNextImageKHR(device_handle, swapchain, 0U, VK_NULL_HANDLE,
                              VK_NULL_HANDLE, &acquired_index);
    if (acquire_result == VK_SUCCESS || acquire_result == VK_SUBOPTIMAL_KHR) {
      acquired_index = acquired_index;
    }

    std::uint32_t image_count_out = 0U;
    const VkResult image_count_result = vkGetSwapchainImagesKHR(
        device_handle, swapchain, &image_count_out, nullptr);
    if (image_count_result != VK_SUCCESS) {
      vkDestroySwapchainKHR(device_handle, swapchain, nullptr);
      vkDestroySurfaceKHR(vk_state->instance(), surface, nullptr);
      return Result<std::unique_ptr<Swapchain>>{
          Error{ErrorCode::swapchain_creation_failed,
                "Vulkan swapchain images could not be queried"}};
    }

    std::vector<VkImage> images(image_count_out);
    if (image_count_out > 0U) {
      const VkResult image_result = vkGetSwapchainImagesKHR(
          device_handle, swapchain, &image_count_out, images.data());
      if (image_result != VK_SUCCESS) {
        vkDestroySwapchainKHR(device_handle, swapchain, nullptr);
        vkDestroySurfaceKHR(vk_state->instance(), surface, nullptr);
        return Result<std::unique_ptr<Swapchain>>{
            Error{ErrorCode::swapchain_creation_failed,
                  "Vulkan swapchain image retrieval failed"}};
      }
    }

    auto queue = std::make_unique<QueueImpl>(
        BackendSelection::vulkan, vk_state->queue(), device_handle, swapchain,
        vk_state->queue_family_index(), VK_NULL_HANDLE, VK_NULL_HANDLE);
    auto instance = std::make_unique<VulkanSwapchain>(
        vk_state->instance(), device_handle, physical_device, vk_state->queue(),
        surface, swapchain, image_count, acquired_index, chosen_format.format,
        chosen_present_mode, capabilities);
    queue->set_semaphores(instance->image_available(),
                          instance->render_finished());
    instance->queue_impl_ = std::move(queue);
    instance->set_image_count(image_count_out);
    instance->set_images(images);
    logger.write(LogLevel::info, LogCategory::graphics,
                 "Vulkan swapchain created successfully");
    return Result<std::unique_ptr<Swapchain>>{std::move(instance)};
  }

  void set_queue(std::unique_ptr<Queue> queue) {
    queue_impl_ = std::move(queue);
  }
  void set_image_count(std::uint32_t count) noexcept { image_count_ = count; }
  void set_images(std::vector<VkImage> images) noexcept {
    images_ = std::move(images);
  }
  void set_semaphores(VkSemaphore image_available,
                      VkSemaphore render_finished) noexcept {
    image_available_ = image_available;
    render_finished_ = render_finished;
  }
  [[nodiscard]] VkSemaphore image_available() const noexcept {
    return image_available_;
  }
  [[nodiscard]] VkSemaphore render_finished() const noexcept {
    return render_finished_;
  }

private:
  VkInstance instance_ = VK_NULL_HANDLE;
  VkDevice device_ = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkQueue queue_ = VK_NULL_HANDLE;
  VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
  std::uint32_t image_count_ = 0U;
  std::uint32_t current_image_index_ = 0U;
  VkFormat format_ = VK_FORMAT_UNDEFINED;
  VkPresentModeKHR present_mode_ = VK_PRESENT_MODE_FIFO_KHR;
  VkSurfaceCapabilitiesKHR capabilities_{};
  VkSemaphore image_available_ = VK_NULL_HANDLE;
  VkSemaphore render_finished_ = VK_NULL_HANDLE;
  std::vector<VkImage> images_{};
  std::unique_ptr<Queue> queue_impl_;
};
#endif

#if defined(MANMENMI_HAS_DX12)
class Dx12Swapchain final : public Swapchain {
public:
  Dx12Swapchain(ID3D12Device *device, ID3D12CommandQueue *queue,
                IDXGISwapChain *swapchain, std::uint32_t width,
                std::uint32_t height)
      : device_(device), queue_(queue), swapchain_(swapchain), width_(width),
        height_(height) {
    if (device_ != nullptr) {
      device_->AddRef();
    }
    if (queue_ != nullptr) {
      queue_->AddRef();
    }
    if (swapchain_ != nullptr) {
      swapchain_->AddRef();
    }
    if (device_ != nullptr) {
      D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
      heap_desc.NumDescriptors = 3U;
      heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
      device_->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&rtv_heap_));
      rtv_increment_ = device_->GetDescriptorHandleIncrementSize(
          D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }
  }

  ~Dx12Swapchain() override {
    if (rtv_heap_ != nullptr) {
      rtv_heap_->Release();
    }
    if (swapchain_ != nullptr) {
      swapchain_->Release();
    }
    if (queue_ != nullptr) {
      queue_->Release();
    }
    if (device_ != nullptr) {
      device_->Release();
    }
  }

  Status acquire_next_image() override {
    if (swapchain_ == nullptr) {
      return Status{
          Error{ErrorCode::not_initialized, "DX12 swapchain is missing"}};
    }

    IDXGISwapChain3 *swapchain3 = nullptr;
    if (SUCCEEDED(swapchain_->QueryInterface(
            __uuidof(IDXGISwapChain3),
            reinterpret_cast<void **>(&swapchain3)))) {
      current_image_index_ = swapchain3->GetCurrentBackBufferIndex();
      swapchain3->Release();
      return success();
    }

    current_image_index_ = 0U;
    return Status{Error{ErrorCode::swapchain_creation_failed,
                        "DX12 swapchain does not expose IDXGISwapChain3"}};
  }

  [[nodiscard]] std::uint32_t image_count() const noexcept override {
    return image_count_;
  }
  [[nodiscard]] std::uint32_t current_image_index() const noexcept override {
    return current_image_index_;
  }
  [[nodiscard]] bool native_runtime_ready_for_test() const noexcept {
    return swapchain_ != nullptr && queue_ != nullptr && !backbuffers_.empty();
  }
  [[nodiscard]] Queue *queue_for_test() noexcept { return queue_impl_.get(); }
  [[nodiscard]] const Queue *queue_for_test() const noexcept {
    return queue_impl_.get();
  }
  [[nodiscard]] ID3D12Resource *current_backbuffer() const noexcept {
    return current_image_index_ < backbuffers_.size()
               ? backbuffers_[current_image_index_]
               : nullptr;
  }
  [[nodiscard]] ID3D12Device *device_handle() const noexcept { return device_; }
  [[nodiscard]] ID3D12CommandQueue *queue_handle() const noexcept {
    return queue_;
  }
  [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE current_rtv() const noexcept {
    auto handle = rtv_heap_ != nullptr
                      ? rtv_heap_->GetCPUDescriptorHandleForHeapStart()
                      : D3D12_CPU_DESCRIPTOR_HANDLE{};
    handle.ptr += static_cast<SIZE_T>(current_image_index_) * rtv_increment_;
    return handle;
  }
  [[nodiscard]] std::uint32_t width() const noexcept { return width_; }
  [[nodiscard]] std::uint32_t height() const noexcept { return height_; }

  Status resize(std::uint32_t width, std::uint32_t height) override {
    if (width == 0U || height == 0U) {
      return Status{Error{ErrorCode::resize_failed,
                          "DX12 swapchain resize requires non-zero extent"}};
    }
    if (swapchain_ == nullptr) {
      return Status{
          Error{ErrorCode::not_initialized, "DX12 swapchain is missing"}};
    }
    if (queue_impl_ == nullptr || !queue_impl_->wait_idle()) {
      return Status{Error{ErrorCode::resize_failed,
                          "DX12 GPU synchronization failed before resize"}};
    }
    release_backbuffers();
    DXGI_SWAP_CHAIN_DESC desc{};
    swapchain_->GetDesc(&desc);
    const HRESULT hr = swapchain_->ResizeBuffers(
        desc.BufferCount, width, height, desc.BufferDesc.Format, desc.Flags);
    if (FAILED(hr)) {
      return Status{
          Error{ErrorCode::resize_failed, "DX12 swapchain resize failed"}};
    }
    width_ = width;
    height_ = height;
    if (FAILED(refresh_backbuffers())) {
      return Status{Error{
          ErrorCode::resize_failed,
          "DX12 swapchain backbuffers could not be refreshed after resize"}};
    }
    return acquire_next_image();
  }

  Status recreate() override {
    if (swapchain_ == nullptr) {
      return Status{
          Error{ErrorCode::not_initialized, "DX12 swapchain is missing"}};
    }
    if (queue_impl_ == nullptr || !queue_impl_->wait_idle()) {
      return Status{Error{ErrorCode::resize_failed,
                          "DX12 GPU synchronization failed before recreate"}};
    }
    release_backbuffers();

    DXGI_SWAP_CHAIN_DESC desc{};
    swapchain_->GetDesc(&desc);
    const HRESULT hr = swapchain_->ResizeBuffers(
        desc.BufferCount, width_, height_, desc.BufferDesc.Format, desc.Flags);
    if (FAILED(hr)) {
      return Status{
          Error{ErrorCode::resize_failed, "DX12 swapchain recreate failed"}};
    }
    if (FAILED(refresh_backbuffers())) {
      return Status{Error{
          ErrorCode::resize_failed,
          "DX12 swapchain backbuffers could not be refreshed after recreate"}};
    }
    return acquire_next_image();
  }

  static Result<std::unique_ptr<Swapchain>>
  create(Device &device, const SurfaceToken &surface_token, std::uint32_t width,
         std::uint32_t height, Logger &logger) {
    auto *concrete = dynamic_cast<DeviceImpl *>(&device);
    if (concrete == nullptr) {
      return Result<std::unique_ptr<Swapchain>>{
          Error{ErrorCode::not_initialized, "device is not a MANMENMI device"}};
    }
    auto *dx12_state = dynamic_cast<Dx12DeviceState *>(concrete->state());
    if (dx12_state == nullptr) {
      return Result<std::unique_ptr<Swapchain>>{
          Error{ErrorCode::unsupported_backend, "device is not DX12"}};
    }

    const auto surface_ref = resolve_surface_token(surface_token);
    if (!surface_ref.valid || surface_ref.window == nullptr) {
      return Result<std::unique_ptr<Swapchain>>{
          Error{ErrorCode::invalid_argument,
                "SurfaceToken does not resolve to a valid window"}};
    }

#if defined(_WIN32)
    HWND hwnd = reinterpret_cast<HWND>(
        SDL_GetPointerProperty(SDL_GetWindowProperties(surface_ref.window),
                               SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
    if (hwnd == nullptr) {
      return Result<std::unique_ptr<Swapchain>>{
          Error{ErrorCode::surface_creation_failed,
                "unable to resolve native HWND from SDL window"}};
    }

    IDXGIFactory *factory = nullptr;
    HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory),
                                    reinterpret_cast<void **>(&factory));
    if (FAILED(hr) || factory == nullptr) {
      return Result<std::unique_ptr<Swapchain>>{Error{
          ErrorCode::surface_creation_failed, "DXGI factory creation failed"}};
    }

    DXGI_SWAP_CHAIN_DESC desc{};
    desc.BufferDesc.Width = width;
    desc.BufferDesc.Height = height;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferDesc.RefreshRate.Numerator = 60U;
    desc.BufferDesc.RefreshRate.Denominator = 1U;
    desc.BufferCount = 2U;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = hwnd;
    desc.Windowed = TRUE;
    desc.SampleDesc.Count = 1U;
    desc.SampleDesc.Quality = 0U;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    IDXGISwapChain *swapchain = nullptr;
    hr = factory->CreateSwapChain(dx12_state->queue(), &desc, &swapchain);
    factory->Release();
    if (FAILED(hr) || swapchain == nullptr) {
      return Result<std::unique_ptr<Swapchain>>{
          Error{ErrorCode::swapchain_creation_failed,
                "DX12 swapchain creation failed"}};
    }

    auto queue = std::make_unique<QueueImpl>(BackendSelection::dx12,
                                             dx12_state->device(),
                                             dx12_state->queue(), swapchain);
    auto instance = std::make_unique<Dx12Swapchain>(
        dx12_state->device(), dx12_state->queue(), swapchain, width, height);
    instance->queue_impl_ = std::move(queue);
    if (FAILED(instance->refresh_backbuffers())) {
      return Result<std::unique_ptr<Swapchain>>{
          Error{ErrorCode::swapchain_creation_failed,
                "DX12 backbuffers could not be initialized"}};
    }
    logger.write(LogLevel::info, LogCategory::graphics,
                 "DX12 swapchain created successfully");
    return Result<std::unique_ptr<Swapchain>>{std::move(instance)};
#else
    (void)logger;
    return Result<std::unique_ptr<Swapchain>>{
        Error{ErrorCode::unsupported_backend,
              "DX12 swapchain creation is only supported on Windows"}};
#endif
  }

  void set_queue(std::unique_ptr<Queue> queue) {
    queue_impl_ = std::move(queue);
  }

  void release_backbuffers() noexcept {
    for (auto *resource : backbuffers_) {
      if (resource != nullptr) {
        resource->Release();
      }
    }
    backbuffers_.clear();
  }

  HRESULT refresh_backbuffers() {
    if (swapchain_ == nullptr) {
      return E_FAIL;
    }
    release_backbuffers();

    DXGI_SWAP_CHAIN_DESC desc{};
    swapchain_->GetDesc(&desc);
    image_count_ = static_cast<std::uint32_t>(desc.BufferCount);
    for (UINT index = 0; index < desc.BufferCount; ++index) {
      ID3D12Resource *resource = nullptr;
      const HRESULT hr = swapchain_->GetBuffer(index, IID_PPV_ARGS(&resource));
      if (FAILED(hr) || resource == nullptr) {
        return hr;
      }
      backbuffers_.push_back(resource);
      if (rtv_heap_ != nullptr) {
        auto handle = rtv_heap_->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += static_cast<SIZE_T>(index) * rtv_increment_;
        device_->CreateRenderTargetView(resource, nullptr, handle);
      }
    }

    IDXGISwapChain3 *swapchain3 = nullptr;
    if (SUCCEEDED(swapchain_->QueryInterface(
            __uuidof(IDXGISwapChain3),
            reinterpret_cast<void **>(&swapchain3)))) {
      current_image_index_ = swapchain3->GetCurrentBackBufferIndex();
      swapchain3->Release();
    }
    return S_OK;
  }

private:
  ID3D12Device *device_ = nullptr;
  ID3D12CommandQueue *queue_ = nullptr;
  IDXGISwapChain *swapchain_ = nullptr;
  std::vector<ID3D12Resource *> backbuffers_{};
  std::uint32_t width_ = 0U;
  std::uint32_t height_ = 0U;
  std::uint32_t image_count_ = 2U;
  std::uint32_t current_image_index_ = 0U;
  ID3D12DescriptorHeap *rtv_heap_ = nullptr;
  UINT rtv_increment_ = 0U;
  std::unique_ptr<Queue> queue_impl_;
};

[[nodiscard]] bool
resolve_dx12_render_target(graphics::Swapchain &swapchain,
                           Dx12RenderTargetBinding &binding) noexcept {
  auto *concrete = dynamic_cast<Dx12Swapchain *>(&swapchain);
  if (concrete == nullptr || concrete->current_backbuffer() == nullptr ||
      concrete->current_rtv().ptr == 0U) {
    return false;
  }
  binding.resource = concrete->current_backbuffer();
  binding.rtv = concrete->current_rtv();
  binding.width = concrete->width();
  binding.height = concrete->height();
  return true;
}

[[nodiscard]] bool
resolve_dx12_render_target(Texture &texture,
                           Dx12RenderTargetBinding &binding) noexcept {
  auto *concrete = dynamic_cast<TextureImpl *>(&texture);
  auto *state = concrete == nullptr
                    ? nullptr
                    : dynamic_cast<Dx12TextureState *>(concrete->state());
  if (state == nullptr || state->resource() == nullptr ||
      state->rtv().ptr == 0U) {
    return false;
  }
  binding.resource = state->resource();
  binding.rtv = state->rtv();
  binding.width = state->width();
  binding.height = state->height();
  binding.texture_target = true;
  return true;
}
#endif

namespace detail {

[[nodiscard]] Queue *runtime_queue_for_test(Swapchain &swapchain) noexcept {
#if defined(MANMENMI_HAS_VULKAN)
  if (auto *vk = dynamic_cast<VulkanSwapchain *>(&swapchain)) {
    return vk->queue_for_test();
  }
#endif
#if defined(MANMENMI_HAS_DX12)
  if (auto *dx = dynamic_cast<Dx12Swapchain *>(&swapchain)) {
    return dx->queue_for_test();
  }
#endif
  return nullptr;
}

[[nodiscard]] bool
runtime_vulkan_handles_valid_for_test(const Swapchain &swapchain) noexcept {
#if defined(MANMENMI_HAS_VULKAN)
  const auto *vk = dynamic_cast<const VulkanSwapchain *>(&swapchain);
  return vk != nullptr && vk->native_runtime_ready_for_test();
#else
  return false;
#endif
}

[[nodiscard]] bool
runtime_dx12_handles_valid_for_test(const Swapchain &swapchain) noexcept {
#if defined(MANMENMI_HAS_DX12)
  const auto *dx = dynamic_cast<const Dx12Swapchain *>(&swapchain);
  return dx != nullptr && dx->native_runtime_ready_for_test();
#else
  return false;
#endif
}

[[nodiscard]] bool runtime_dx12_render_target_has_triangle_for_test(
    Swapchain &swapchain) noexcept {
#if defined(MANMENMI_HAS_DX12)
  auto *concrete = dynamic_cast<Dx12Swapchain *>(&swapchain);
  if (concrete == nullptr || concrete->device_handle() == nullptr ||
      concrete->queue_handle() == nullptr ||
      concrete->current_backbuffer() == nullptr) {
    return false;
  }
  auto *device = concrete->device_handle();
  auto *queue = concrete->queue_handle();
  auto *source = concrete->current_backbuffer();
  D3D12_RESOURCE_DESC source_desc = source->GetDesc();
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
  UINT row_count = 0U;
  UINT64 row_size = 0U;
  UINT64 total_size = 0U;
  device->GetCopyableFootprints(&source_desc, 0U, 1U, 0U, &footprint,
                                &row_count, &row_size, &total_size);
  if (total_size == 0U || footprint.Footprint.RowPitch == 0U) {
    return false;
  }

  D3D12_HEAP_PROPERTIES heap_properties{};
  heap_properties.Type = D3D12_HEAP_TYPE_READBACK;
  D3D12_RESOURCE_DESC readback_desc{};
  readback_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  readback_desc.Width = total_size;
  readback_desc.Height = 1U;
  readback_desc.DepthOrArraySize = 1U;
  readback_desc.MipLevels = 1U;
  readback_desc.Format = DXGI_FORMAT_UNKNOWN;
  readback_desc.SampleDesc.Count = 1U;
  readback_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  ID3D12Resource *readback = nullptr;
  if (FAILED(device->CreateCommittedResource(
          &heap_properties, D3D12_HEAP_FLAG_NONE, &readback_desc,
          D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&readback))) ||
      readback == nullptr) {
    return false;
  }

  ID3D12CommandAllocator *allocator = nullptr;
  ID3D12GraphicsCommandList *list = nullptr;
  ID3D12Fence *fence = nullptr;
  HANDLE event = nullptr;
  bool result = false;
  do {
    if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                              IID_PPV_ARGS(&allocator))) ||
        allocator == nullptr) {
      break;
    }
    if (FAILED(device->CreateCommandList(0U, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                         allocator, nullptr,
                                         IID_PPV_ARGS(&list))) ||
        list == nullptr) {
      break;
    }
    D3D12_RESOURCE_BARRIER to_copy{};
    to_copy.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    to_copy.Transition.pResource = source;
    to_copy.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    to_copy.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
    to_copy.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    list->ResourceBarrier(1U, &to_copy);
    D3D12_TEXTURE_COPY_LOCATION source_location{};
    source_location.pResource = source;
    source_location.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    source_location.SubresourceIndex = 0U;
    D3D12_TEXTURE_COPY_LOCATION destination_location{};
    destination_location.pResource = readback;
    destination_location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    destination_location.PlacedFootprint = footprint;
    list->CopyTextureRegion(&destination_location, 0U, 0U, 0U, &source_location,
                            nullptr);
    to_copy.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
    to_copy.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    list->ResourceBarrier(1U, &to_copy);
    if (FAILED(list->Close())) {
      break;
    }
    ID3D12CommandList *command_lists[] = {list};
    queue->ExecuteCommandLists(1U, command_lists);
    if (FAILED(device->CreateFence(0ULL, D3D12_FENCE_FLAG_NONE,
                                   IID_PPV_ARGS(&fence))) ||
        fence == nullptr) {
      break;
    }
    event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (event == nullptr || FAILED(queue->Signal(fence, 1ULL)) ||
        FAILED(fence->SetEventOnCompletion(1ULL, event)) ||
        WaitForSingleObject(event, INFINITE) != WAIT_OBJECT_0) {
      break;
    }
    void *mapped = nullptr;
    const HRESULT map_hr = readback->Map(0U, nullptr, &mapped);
    if (FAILED(map_hr) || mapped == nullptr) {
      break;
    }
    const auto *bytes = static_cast<const std::uint8_t *>(mapped);
    for (UINT row = 0U; row < row_count && !result; ++row) {
      const auto *pixel_row =
          bytes + footprint.Offset +
          static_cast<std::size_t>(row) * footprint.Footprint.RowPitch;
      for (UINT column = 0U; column < footprint.Footprint.Width; ++column) {
        const auto *pixel = pixel_row + static_cast<std::size_t>(column) * 4U;
        if (pixel[0] > 40U || pixel[1] > 40U || pixel[2] > 40U) {
          result = true;
          break;
        }
      }
    }
    readback->Unmap(0U, nullptr);
  } while (false);
  if (event != nullptr) {
    CloseHandle(event);
  }
  if (fence != nullptr) {
    fence->Release();
  }
  if (list != nullptr) {
    list->Release();
  }
  if (allocator != nullptr) {
    allocator->Release();
  }
  readback->Release();
  return result;
#else
  (void)swapchain;
  return false;
#endif
}

[[nodiscard]] bool
runtime_dx12_texture_has_triangle_for_test(Texture &texture,
                                           Swapchain &swapchain) noexcept {
#if defined(MANMENMI_HAS_DX12)
  auto *swapchain_concrete = dynamic_cast<Dx12Swapchain *>(&swapchain);
  auto *texture_concrete = dynamic_cast<TextureImpl *>(&texture);
  auto *texture_state =
      texture_concrete == nullptr
          ? nullptr
          : dynamic_cast<Dx12TextureState *>(texture_concrete->state());
  if (swapchain_concrete == nullptr || texture_state == nullptr ||
      swapchain_concrete->device_handle() == nullptr ||
      swapchain_concrete->queue_handle() == nullptr ||
      texture_state->resource() == nullptr) {
    return false;
  }
  auto *device = swapchain_concrete->device_handle();
  auto *queue = swapchain_concrete->queue_handle();
  auto *source = texture_state->resource();
  const auto source_desc = source->GetDesc();
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
  UINT row_count = 0U;
  UINT64 row_size = 0U;
  UINT64 total_size = 0U;
  device->GetCopyableFootprints(&source_desc, 0U, 1U, 0U, &footprint,
                                &row_count, &row_size, &total_size);
  if (total_size == 0U || footprint.Footprint.RowPitch == 0U) {
    return false;
  }

  D3D12_HEAP_PROPERTIES heap_properties{};
  heap_properties.Type = D3D12_HEAP_TYPE_READBACK;
  D3D12_RESOURCE_DESC readback_desc{};
  readback_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  readback_desc.Width = total_size;
  readback_desc.Height = 1U;
  readback_desc.DepthOrArraySize = 1U;
  readback_desc.MipLevels = 1U;
  readback_desc.Format = DXGI_FORMAT_UNKNOWN;
  readback_desc.SampleDesc.Count = 1U;
  readback_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  ID3D12Resource *readback = nullptr;
  if (FAILED(device->CreateCommittedResource(
          &heap_properties, D3D12_HEAP_FLAG_NONE, &readback_desc,
          D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&readback))) ||
      readback == nullptr) {
    return false;
  }

  ID3D12CommandAllocator *allocator = nullptr;
  ID3D12GraphicsCommandList *list = nullptr;
  ID3D12Fence *fence = nullptr;
  HANDLE event = nullptr;
  bool result = false;
  do {
    if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                              IID_PPV_ARGS(&allocator))) ||
        allocator == nullptr) {
      break;
    }
    if (FAILED(device->CreateCommandList(0U, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                         allocator, nullptr,
                                         IID_PPV_ARGS(&list))) ||
        list == nullptr) {
      break;
    }
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = source;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    list->ResourceBarrier(1U, &barrier);
    D3D12_TEXTURE_COPY_LOCATION source_location{};
    source_location.pResource = source;
    source_location.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    D3D12_TEXTURE_COPY_LOCATION destination_location{};
    destination_location.pResource = readback;
    destination_location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    destination_location.PlacedFootprint = footprint;
    list->CopyTextureRegion(&destination_location, 0U, 0U, 0U, &source_location,
                            nullptr);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    list->ResourceBarrier(1U, &barrier);
    if (FAILED(list->Close())) {
      break;
    }
    ID3D12CommandList *command_lists[] = {list};
    queue->ExecuteCommandLists(1U, command_lists);
    if (FAILED(device->CreateFence(0ULL, D3D12_FENCE_FLAG_NONE,
                                   IID_PPV_ARGS(&fence))) ||
        fence == nullptr) {
      break;
    }
    event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (event == nullptr || FAILED(queue->Signal(fence, 1ULL)) ||
        FAILED(fence->SetEventOnCompletion(1ULL, event)) ||
        WaitForSingleObject(event, INFINITE) != WAIT_OBJECT_0) {
      break;
    }
    void *mapped = nullptr;
    if (FAILED(readback->Map(0U, nullptr, &mapped)) || mapped == nullptr) {
      break;
    }
    const auto *bytes = static_cast<const std::uint8_t *>(mapped);
    for (UINT row = 0U; row < row_count && !result; ++row) {
      const auto *pixel_row =
          bytes + footprint.Offset +
          static_cast<std::size_t>(row) * footprint.Footprint.RowPitch;
      for (UINT column = 0U; column < footprint.Footprint.Width; ++column) {
        const auto *pixel = pixel_row + static_cast<std::size_t>(column) * 4U;
        if (pixel[0] > 40U || pixel[1] > 40U || pixel[2] > 40U) {
          result = true;
          break;
        }
      }
    }
    readback->Unmap(0U, nullptr);
  } while (false);
  if (event != nullptr) {
    CloseHandle(event);
  }
  if (fence != nullptr) {
    fence->Release();
  }
  if (list != nullptr) {
    list->Release();
  }
  if (allocator != nullptr) {
    allocator->Release();
  }
  readback->Release();
  return result;
#else
  (void)texture;
  (void)swapchain;
  return false;
#endif
}

[[nodiscard]] bool
runtime_dx12_buffer_handle_valid_for_test(const Buffer &buffer) noexcept {
#if defined(MANMENMI_HAS_DX12)
  const auto *concrete = dynamic_cast<const BufferImpl *>(&buffer);
  if (concrete == nullptr || concrete->state() == nullptr) {
    return false;
  }
  const auto *state = dynamic_cast<const Dx12BufferState *>(concrete->state());
  return state != nullptr && state->resource() != nullptr;
#else
  (void)buffer;
  return false;
#endif
}

[[nodiscard]] bool runtime_dx12_buffer_data_matches_for_test(
    const Buffer &buffer, const std::vector<std::uint8_t> &expected) noexcept {
#if defined(MANMENMI_HAS_DX12)
  const auto *concrete = dynamic_cast<const BufferImpl *>(&buffer);
  if (concrete == nullptr || concrete->state() == nullptr) {
    return false;
  }
  const auto *state = dynamic_cast<const Dx12BufferState *>(concrete->state());
  if (state == nullptr || state->resource() == nullptr || expected.empty()) {
    return false;
  }
  void *mapped = nullptr;
  const D3D12_RANGE read_range{0U, static_cast<SIZE_T>(expected.size())};
  const HRESULT map_hr = state->resource()->Map(0U, &read_range, &mapped);
  if (FAILED(map_hr) || mapped == nullptr) {
    return false;
  }
  std::vector<std::uint8_t> actual(expected.size(), 0U);
  std::memcpy(actual.data(), mapped, expected.size());
  state->resource()->Unmap(0U, nullptr);
  return actual == expected;
#else
  (void)buffer;
  (void)expected;
  return false;
#endif
}

[[nodiscard]] bool runtime_dx12_write_buffer_for_test(
    Buffer &buffer, const std::vector<std::uint8_t> &data) noexcept {
#if defined(MANMENMI_HAS_DX12)
  auto *concrete = dynamic_cast<BufferImpl *>(&buffer);
  if (concrete == nullptr || concrete->state() == nullptr) {
    return false;
  }
  auto *state = dynamic_cast<Dx12BufferState *>(concrete->state());
  if (state == nullptr || state->resource() == nullptr ||
      data.size() > concrete->descriptor().size) {
    return false;
  }
  void *mapped = nullptr;
  const D3D12_RANGE read_range{0U, static_cast<SIZE_T>(data.size())};
  const HRESULT map_hr = state->resource()->Map(0U, &read_range, &mapped);
  if (FAILED(map_hr) || mapped == nullptr) {
    return false;
  }
  std::memcpy(mapped, data.data(), data.size());
  state->resource()->Unmap(0U, nullptr);
  return true;
#else
  (void)buffer;
  (void)data;
  return false;
#endif
}

[[nodiscard]] bool runtime_dx12_command_buffer_handle_valid_for_test(
    const CommandBuffer &command_buffer) noexcept {
#if defined(MANMENMI_HAS_DX12)
  const auto *concrete =
      dynamic_cast<const CommandBufferImpl *>(&command_buffer);
  const auto *state =
      concrete != nullptr
          ? dynamic_cast<const Dx12CommandBufferState *>(concrete->state())
          : nullptr;
  return state != nullptr && state->native_list() != nullptr;
#else
  (void)command_buffer;
  return false;
#endif
}

} // namespace detail

class GraphicsFactoryImpl final : public GraphicsFactory {
public:
  [[nodiscard]] Result<std::unique_ptr<Device>>
  create_device(BackendSelection selection, Logger &logger) override {
    const auto candidates = detail::ordered_candidates(selection);
    if (candidates.empty()) {
      logger.write(LogLevel::warning, LogCategory::graphics,
                   "no backend candidates are available on this platform");
      return Result<std::unique_ptr<Device>>{
          Error{ErrorCode::unsupported_backend,
                "graphics backend selection unsupported on this platform"}};
    }

    for (const auto candidate : candidates) {
      const auto probe = detail::probe_backend(candidate);
      const auto label = detail::backend_name(candidate);
      logger.write(LogLevel::info, LogCategory::graphics,
                   std::string{"backend requested: "}.append(label));
      if (!probe.available) {
        logger.write(LogLevel::warning, LogCategory::graphics,
                     std::string{"backend unavailable: "}
                         .append(label)
                         .append(" (reason: ")
                         .append(probe.reason)
                         .append(")"));
        if (selection != BackendSelection::automatic) {
          return Result<std::unique_ptr<Device>>{
              Error{ErrorCode::unsupported_backend,
                    std::string{"backend "}.append(label).append(
                        " is not available")}};
        }
        continue;
      }

      Result<std::unique_ptr<DeviceState>> state_result{Error{
          ErrorCode::unavailable, "backend initialization not attempted"}};
      switch (candidate) {
      case BackendSelection::vulkan:
#if defined(MANMENMI_HAS_VULKAN)
        state_result = VulkanDeviceState::create(logger);
#else
        state_result = Result<std::unique_ptr<DeviceState>>{
            Error{ErrorCode::unavailable, "Vulkan headers are not available"}};
#endif
        break;
      case BackendSelection::dx12:
#if defined(MANMENMI_HAS_DX12)
        state_result = Dx12DeviceState::create(logger);
#else
        state_result = Result<std::unique_ptr<DeviceState>>{
            Error{ErrorCode::unavailable, "DX12 headers are not available"}};
#endif
        break;
      case BackendSelection::opengl:
        state_result = Result<std::unique_ptr<DeviceState>>{
            Error{ErrorCode::unavailable,
                  "OpenGL runtime context creation is intentionally deferred; "
                  "no fake device is created"}};
        break;
      case BackendSelection::automatic:
        state_result = Result<std::unique_ptr<DeviceState>>{
            Error{ErrorCode::unavailable,
                  "automatic selection should never reach this branch"}};
        break;
      }

      if (!state_result) {
        logger.write(LogLevel::warning, LogCategory::graphics,
                     std::string{"backend creation failed: "}
                         .append(label)
                         .append(" (reason: ")
                         .append(state_result.error().message)
                         .append(")"));
        if (selection != BackendSelection::automatic) {
          return Result<std::unique_ptr<Device>>{
              Error{ErrorCode::device_creation_failed,
                    std::string{"backend "}
                        .append(label)
                        .append(" failed to initialize: ")
                        .append(state_result.error().message)}};
        }
        continue;
      }

      auto capabilities = detail::default_capabilities(candidate);
      capabilities.presentable = (candidate == BackendSelection::vulkan ||
                                  candidate == BackendSelection::dx12);
      capabilities.render_target_supported = capabilities.presentable;
      capabilities.multi_buffering = capabilities.presentable;
      capabilities.msaa_supported = false;
      capabilities.separate_graphics_queue = false;
      capabilities.max_backbuffers = 3U;
      auto device = std::make_unique<DeviceImpl>(
          candidate, capabilities, std::move(state_result).value());
      logger.write(
          LogLevel::info, LogCategory::graphics,
          std::string{"native device created for backend: "}.append(label));
      return Result<std::unique_ptr<Device>>{std::move(device)};
    }

    const auto requested = detail::backend_name(
        selection == BackendSelection::automatic ? candidates.front()
                                                 : selection);
    return Result<std::unique_ptr<Device>>{
        Error{ErrorCode::unavailable,
              std::string{"all graphics backends are unavailable for request: "}
                  .append(requested)}};
  }

  [[nodiscard]] Result<std::unique_ptr<Swapchain>>
  create_swapchain(Device &device, const SurfaceToken &surface_token,
                   std::uint32_t width, std::uint32_t height) override {
    switch (device.backend()) {
    case BackendSelection::vulkan:
#if defined(MANMENMI_HAS_VULKAN)
      return VulkanSwapchain::create(device, surface_token, width, height,
                                     logger_);
#else
      return Result<std::unique_ptr<Swapchain>>{
          Error{ErrorCode::unsupported_backend,
                "Vulkan swapchain is not available in this build"}};
#endif
    case BackendSelection::dx12:
#if defined(MANMENMI_HAS_DX12)
      return Dx12Swapchain::create(device, surface_token, width, height,
                                   logger_);
#else
      return Result<std::unique_ptr<Swapchain>>{
          Error{ErrorCode::unsupported_backend,
                "DX12 swapchain is not available in this build"}};
#endif
    case BackendSelection::opengl:
      return Result<std::unique_ptr<Swapchain>>{
          Error{ErrorCode::unavailable,
                "OpenGL swapchain creation is intentionally deferred"}};
    case BackendSelection::automatic:
      return Result<std::unique_ptr<Swapchain>>{
          Error{ErrorCode::invalid_argument,
                "Automatic backend cannot create a swapchain without a "
                "concrete device backend"}};
    }
    return Result<std::unique_ptr<Swapchain>>{Error{
        ErrorCode::unsupported_backend, "requested backend is unsupported"}};
  }

private:
  Logger logger_{std::clog};
};

std::unique_ptr<GraphicsFactory> make_graphics_factory() {
  return std::make_unique<GraphicsFactoryImpl>();
}

} // namespace graphics
} // namespace manmenmi

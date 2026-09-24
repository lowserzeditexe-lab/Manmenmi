# API coverage — portable graphics surface

| Feature | Public API | DX12 | Vulkan | OpenGL |
|---|---|---|---|---|
| Device / Queue / Swapchain | VALIDATED | VALIDATED M1.3 | NOT VALIDATED | NOT IMPLEMENTED |
| Buffer / BufferDescriptor | VALIDATED | VALIDATED M2.1 | NOT IMPLEMENTED | NOT IMPLEMENTED |
| CommandBuffer / CopyBuffer | VALIDATED | VALIDATED M2.2 | NOT IMPLEMENTED | NOT IMPLEMENTED |
| Private ordered transfer IR | INTERNAL | VALIDATED M2.3 | NOT VALIDATED | NOT VALIDATED |
| Minimal private Shader IR | INTERNAL | VALIDATED M6 | NOT IMPLEMENTED | NOT IMPLEMENTED |
| Minimal Pipeline | VALIDATED | VALIDATED M3 | NOT IMPLEMENTED | NOT IMPLEMENTED |
| Render pass / vertex binding / draw | VALIDATED | VALIDATED M3 | NOT IMPLEMENTED | NOT IMPLEMENTED |
| Texture RGBA8 render target | VALIDATED | VALIDATED M7 | NOT IMPLEMENTED | NOT IMPLEMENTED |
| GX2 R8_G8_B8_A8 UNORM color-target descriptor mapping | PARTIAL: documented enum, inferred semantic projection | VALIDATED M8 descriptor bridge | NOT IMPLEMENTED | NOT IMPLEMENTED |
| Public Shader / Texture / Framebuffer / Sync | ABSENT | OUT OF SCOPE | OUT OF SCOPE | OUT OF SCOPE |

M3 uses two host-native HLSL shaders compiled privately with `D3DCompile`.
M6 now generates those HLSL sources from a small private backend-neutral IR
before `D3DCompile`. This is a validated micro-subset, not GX2 shader support,
shader translation completeness, or a cross-backend shader contract. The M3
runtime performs a private readback to verify non-clear triangle pixels. M7
supports only one offscreen RGBA8 render-target texture. M8 maps one documented
GX2 R8_G8_B8_A8 UNORM color-target descriptor to that resource. Its accepted
`LINEAR_ALIGNED` tile-mode value is descriptive only: GX2 pitch, alignment,
swizzle, and data-layout conversion remain unimplemented.

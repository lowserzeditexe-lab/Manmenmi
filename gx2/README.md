# GX2 - M8 texture/layout foundation

M8 adds one evidence-bounded GX2 texture descriptor above the existing graphics
abstraction. The accepted subset is a two-dimensional color target with
`GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8` (`0x01a`),
`GX2_SURFACE_USE_COLOR_BUFFER`, and `GX2_TILE_MODE_LINEAR_ALIGNED` (`1`).
Those names and values are DOCUMENTED by the pinned WUT headers in
[SOURCES](../docs/research/SOURCES.md).

`gx2::Texture` is an opaque RAII owner of the resulting `manmenmi::Texture`.
The explicit conversion is:

```text
GX2 descriptor
  -> MANMENMI TextureDescriptor(rgba8_unorm, render_target,
                                opaque_render_target)
  -> MANMENMI Texture
  -> backend texture
```

The R8/G8/B8/A8 UNORM name is DOCUMENTED. Its semantic projection to the
MANMENMI RGBA8 target is INFERRED and is tested by the DX12 rendering path. It
does not establish console byte order or data compatibility.

`LINEAR_ALIGNED` is accepted only as descriptive metadata. M8 does not import
GX2 image data and implements no pitch, alignment, tiling, swizzle, cache, or
memory conversion. These details remain UNKNOWN. Tiled modes, all other
formats and usages, depth/stencil, MSAA, mipmaps, arrays, cube maps, GX2 ABI,
Cafe, Latte, and game compatibility remain out of scope.

DX12 is IMPLEMENTED + VALIDATED by `runtime.integration_m8`; Vulkan and OpenGL
are NOT IMPLEMENTED / NOT VALIDATED. The public GX2 API exposes no native DX12,
Vulkan, OpenGL, or SDL type. See the
[GX2 coverage matrix](../docs/gx2/api-coverage.md).

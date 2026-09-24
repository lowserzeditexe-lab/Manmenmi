#include "check.hpp"

#include "../graphics/src/shader_ir.hpp"

#include <d3dcompiler.h>

int main() {
    using namespace manmenmi::graphics::shader_ir;

    const auto vertex_result = translate_to_hlsl(triangle_vertex_module());
    const auto pixel_result = translate_to_hlsl(triangle_pixel_module());
    CHECK(vertex_result.has_value());
    CHECK(pixel_result.has_value());

    ID3DBlob* vertex_blob = nullptr;
    ID3DBlob* pixel_blob = nullptr;
    ID3DBlob* errors = nullptr;
    const auto vertex_compile = D3DCompile(
        vertex_result.value().source.data(), vertex_result.value().source.size(),
        "m6_vertex", nullptr, nullptr, "main", "vs_5_0",
        D3DCOMPILE_ENABLE_STRICTNESS, 0U, &vertex_blob, &errors);
    if (errors != nullptr) { errors->Release(); }
    CHECK(SUCCEEDED(vertex_compile));
    CHECK(vertex_blob != nullptr);
    if (vertex_blob != nullptr) { vertex_blob->Release(); }

    const auto pixel_compile = D3DCompile(
        pixel_result.value().source.data(), pixel_result.value().source.size(),
        "m6_pixel", nullptr, nullptr, "main", "ps_5_0",
        D3DCOMPILE_ENABLE_STRICTNESS, 0U, &pixel_blob, &errors);
    if (errors != nullptr) { errors->Release(); }
    CHECK(SUCCEEDED(pixel_compile));
    CHECK(pixel_blob != nullptr);
    if (pixel_blob != nullptr) { pixel_blob->Release(); }

    return checks::finish();
}

#include "check.hpp"

#include "../graphics/src/shader_ir.hpp"

#include <string>

int main() {
    using namespace manmenmi::graphics::shader_ir;

    const auto vertex_module = triangle_vertex_module();
    const auto pixel_module = triangle_pixel_module();
    CHECK(validate(vertex_module));
    CHECK(validate(pixel_module));

    const auto vertex_source = translate_to_hlsl(vertex_module);
    const auto pixel_source = translate_to_hlsl(pixel_module);
    CHECK(vertex_source.has_value());
    CHECK(pixel_source.has_value());
    CHECK(vertex_source.value().source == translate_to_hlsl(vertex_module).value().source);
    CHECK(vertex_source.value().source.find("input.position") != std::string::npos);
    CHECK(pixel_source.value().source.find("SV_TARGET") != std::string::npos);

    auto invalid_operand = vertex_module;
    invalid_operand.instructions[2].operand_a = 99U;
    CHECK(!validate(invalid_operand));

    auto invalid_output = pixel_module;
    invalid_output.instructions.pop_back();
    CHECK(!validate(invalid_output));

    auto invalid_opcode = vertex_module;
    invalid_opcode.instructions[0].opcode = static_cast<OpCode>(99);
    CHECK(!validate(invalid_opcode));

    auto invalid_stage = pixel_module;
    invalid_stage.stage = static_cast<Stage>(99);
    CHECK(!validate(invalid_stage));

    Module arithmetic_module{Stage::pixel, {
        {OpCode::input_color, 1U},
        {OpCode::constant_float4, 2U, 0U, 0U, {0.1F, 0.2F, 0.3F, 0.4F}},
        {OpCode::add_float4, 3U, 1U, 2U},
        {OpCode::output_color, 0U, 3U},
    }};
    CHECK(validate(arithmetic_module));
    const auto arithmetic_source = translate_to_hlsl(arithmetic_module);
    CHECK(arithmetic_source.has_value());
    CHECK(arithmetic_source.value().source.find(" + ") != std::string::npos);

    return checks::finish();
}

#pragma once

#include <array>
#include <cstdint>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <manmenmi/core/result.hpp>

namespace manmenmi::graphics::shader_ir {

enum class Stage {
    vertex,
    pixel
};

enum class ValueType {
    float2,
    float4
};

enum class OpCode {
    input_position,
    input_color,
    constant_float4,
    add_float4,
    make_position,
    move,
    output_position,
    output_color
};

struct Instruction {
    OpCode opcode;
    std::uint32_t result = 0U;
    std::uint32_t operand_a = 0U;
    std::uint32_t operand_b = 0U;
    std::array<float, 4> literal{};
};

struct Module {
    Stage stage;
    std::vector<Instruction> instructions;
};

struct HlslSource {
    Stage stage;
    std::string source;
};

inline Status validate(const Module& module) {
    if (module.stage != Stage::vertex && module.stage != Stage::pixel) {
        return Status{Error{ErrorCode::invalid_argument, "shader IR stage is invalid"}};
    }
    std::vector<ValueType> values(1U);
    bool has_position_output = false;
    bool has_color_output = false;
    for (const auto& instruction : module.instructions) {
        const auto has_value = [&](std::uint32_t value) {
            return value != 0U && value < values.size();
        };
        const auto define_value = [&](std::uint32_t value, ValueType type) -> Status {
            if (value == 0U || value != values.size()) {
                return Status{Error{ErrorCode::invalid_argument,
                    "shader IR result ids must be non-zero and sequential"}};
            }
            values.push_back(type);
            return success();
        };
        Status status = success();
        switch (instruction.opcode) {
        case OpCode::input_position:
            if (module.stage != Stage::vertex) {
                return Status{Error{ErrorCode::invalid_argument,
                    "position input is only valid in the vertex stage"}};
            }
            status = define_value(instruction.result, ValueType::float2);
            break;
        case OpCode::input_color:
            status = define_value(instruction.result, ValueType::float4);
            break;
        case OpCode::constant_float4:
            status = define_value(instruction.result, ValueType::float4);
            break;
        case OpCode::add_float4:
            if (!has_value(instruction.operand_a) || !has_value(instruction.operand_b) ||
                values[instruction.operand_a] != ValueType::float4 ||
                values[instruction.operand_b] != ValueType::float4) {
                return Status{Error{ErrorCode::invalid_argument,
                    "shader IR add requires two float4 operands"}};
            }
            status = define_value(instruction.result, ValueType::float4);
            break;
        case OpCode::make_position:
            if (module.stage != Stage::vertex || !has_value(instruction.operand_a) ||
                values[instruction.operand_a] != ValueType::float2) {
                return Status{Error{ErrorCode::invalid_argument,
                    "shader IR position construction requires a vertex float2"}};
            }
            status = define_value(instruction.result, ValueType::float4);
            break;
        case OpCode::move:
            if (!has_value(instruction.operand_a) ||
                values[instruction.operand_a] != ValueType::float4) {
                return Status{Error{ErrorCode::invalid_argument,
                    "shader IR move requires a defined float4 operand"}};
            }
            status = define_value(instruction.result, ValueType::float4);
            break;
        case OpCode::output_position:
            if (module.stage != Stage::vertex || !has_value(instruction.operand_a) ||
                values[instruction.operand_a] != ValueType::float4) {
                return Status{Error{ErrorCode::invalid_argument,
                    "shader IR position output requires a vertex float4"}};
            }
            has_position_output = true;
            break;
        case OpCode::output_color:
            if (!has_value(instruction.operand_a) ||
                values[instruction.operand_a] != ValueType::float4) {
                return Status{Error{ErrorCode::invalid_argument,
                    "shader IR color output requires a float4"}};
            }
            has_color_output = true;
            break;
        default:
            return Status{Error{ErrorCode::unimplemented,
                "shader IR opcode is not supported"}};
        }
        if (!status) {
            return status;
        }
    }
    if (module.stage == Stage::vertex && (!has_position_output || !has_color_output)) {
        return Status{Error{ErrorCode::invalid_argument,
            "vertex shader IR requires position and color outputs"}};
    }
    if (module.stage == Stage::pixel && !has_color_output) {
        return Status{Error{ErrorCode::invalid_argument,
            "pixel shader IR requires a color output"}};
    }
    return success();
}

inline Result<HlslSource> translate_to_hlsl(const Module& module) {
    auto validation = validate(module);
    if (!validation) {
        return Result<HlslSource>{validation.error()};
    }
    std::ostringstream source;
    source.imbue(std::locale::classic());
    source << std::setprecision(9);
    if (module.stage == Stage::vertex) {
        source << "struct VSInput { float2 position : POSITION; float4 color : COLOR; };\n"
               << "struct VSOutput { float4 position : SV_POSITION; float4 color : COLOR; };\n"
               << "VSOutput main(VSInput input) { VSOutput output;\n";
    } else {
        source << "struct PSInput { float4 position : SV_POSITION; float4 color : COLOR; };\n"
               << "float4 main(PSInput input) : SV_TARGET {\n";
    }
    const auto value_name = [](std::uint32_t value) {
        return std::string{"v"}.append(std::to_string(value));
    };
    std::string position_output;
    std::string color_output;
    for (const auto& instruction : module.instructions) {
        switch (instruction.opcode) {
        case OpCode::input_position:
            source << "float2 " << value_name(instruction.result) << " = input.position;\n";
            break;
        case OpCode::input_color:
            source << "float4 " << value_name(instruction.result) << " = input.color;\n";
            break;
        case OpCode::constant_float4:
            source << "float4 " << value_name(instruction.result) << " = float4("
                   << instruction.literal[0] << "f, " << instruction.literal[1] << "f, "
                   << instruction.literal[2] << "f, " << instruction.literal[3] << "f);\n";
            break;
        case OpCode::add_float4:
            source << "float4 " << value_name(instruction.result) << " = "
                   << value_name(instruction.operand_a) << " + "
                   << value_name(instruction.operand_b) << ";\n";
            break;
        case OpCode::make_position:
            source << "float4 " << value_name(instruction.result) << " = float4("
                   << value_name(instruction.operand_a) << ", 0.0f, 1.0f);\n";
            break;
        case OpCode::move:
            source << "float4 " << value_name(instruction.result) << " = "
                   << value_name(instruction.operand_a) << ";\n";
            break;
        case OpCode::output_position:
            position_output = value_name(instruction.operand_a);
            break;
        case OpCode::output_color:
            color_output = value_name(instruction.operand_a);
            break;
        default:
            return Result<HlslSource>{Error{ErrorCode::unimplemented,
                "shader IR opcode translation is not supported"}};
        }
    }
    if (module.stage == Stage::vertex) {
        source << "output.position = " << position_output << ";\n"
               << "output.color = " << color_output << "; return output; }\n";
    } else {
        source << "return " << color_output << "; }\n";
    }
    return Result<HlslSource>{HlslSource{module.stage, source.str()}};
}

inline Module triangle_vertex_module() {
    return Module{Stage::vertex, {
        {OpCode::input_position, 1U},
        {OpCode::input_color, 2U},
        {OpCode::make_position, 3U, 1U},
        {OpCode::output_position, 0U, 3U},
        {OpCode::output_color, 0U, 2U},
    }};
}

inline Module triangle_pixel_module() {
    return Module{Stage::pixel, {
        {OpCode::input_color, 1U},
        {OpCode::output_color, 0U, 1U},
    }};
}

} // namespace manmenmi::graphics::shader_ir

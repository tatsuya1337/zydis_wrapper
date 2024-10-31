#pragma once

#include "encoded_instructions.hpp"
#include <vector>
#include <variant>
#include <tuple>
#include <utility>

namespace zydis {
    namespace encoder {

        using register_operand = ZydisRegister;
        using memory_operand = std::tuple<ZydisRegister, ZydisRegister, uint8_t, int64_t, uint16_t>;
        using pointer_operand = std::pair<uint16_t, uint32_t>;
        using immediate_operand = uint64_t;

        using operand = std::variant<register_operand, memory_operand, pointer_operand, immediate_operand>;

        encoded_instructions encode(const ZydisEncoderRequest& request);
        ZydisInstructionAttributes get_segment_prefix(ZydisRegister reg);
        encoded_instructions encode(const std::vector<std::pair<ZydisMnemonic, std::vector<operand>>>& instructions);
        encoded_instructions encode_absolute_jump(uintptr_t target);
        encoded_instructions encode_absolute_call(uintptr_t target);

    }
}

#pragma once
#include "../instruction/single_instruction.hpp"
#include <functional>

namespace zydis {
    namespace find_predicates {

        inline const InstructionCondition is_epilogue = FIND_SIGNATURE_LAMBDA
        {
            return (instruction.info().mnemonic == ZYDIS_MNEMONIC_ADD) &&
                   (instruction.operand(0).reg.value == ZYDIS_REGISTER_RSP);
        };

        inline const InstructionCondition is_int3 = FIND_SIGNATURE_LAMBDA
        {
            return instruction.info().mnemonic == ZYDIS_MNEMONIC_INT3;
        };

        inline const InstructionCondition is_memory_reference_to(uintptr_t target_address) {
            return [=](const single_instruction& instruction) -> bool {
                if (instruction.info().mnemonic == ZYDIS_MNEMONIC_CALL)
                    return false;

                for (uint8_t i = 0; i < instruction.info().operand_count_visible; ++i) {
                    ZydisDecodedOperand op = instruction.operand(i);

                    if (op.type == ZYDIS_OPERAND_TYPE_MEMORY && op.mem.base == ZYDIS_REGISTER_RIP)
                        return instruction.relative_to_absolute() == target_address;

                }

                return false;
                };
        }

        inline const InstructionCondition is_call_to(uintptr_t target_address) {
            return [=](const single_instruction& instruction) -> bool {
                if (instruction.info().mnemonic != ZYDIS_MNEMONIC_CALL)
                    return false;

                auto call_address = instruction.relative_to_absolute();

                if (!call_address)
                    return false;

                for (uint8_t i = 0; i < instruction.info().operand_count_visible; ++i) {
                    ZydisDecodedOperand op = instruction.operand(i);

                    if (op.type == ZYDIS_OPERAND_TYPE_MEMORY && op.mem.base == ZYDIS_REGISTER_RIP)
                    {
                        call_address = *(uintptr_t*)call_address;
                        break;
                    }

                }

                return call_address == target_address;
                };
        }

        inline const InstructionCondition is_jump_to(uintptr_t target_address) {
            return [=](const single_instruction& instruction) -> bool {
                if (instruction.info().meta.category != ZYDIS_CATEGORY_COND_BR &&
                    instruction.info().meta.category != ZYDIS_CATEGORY_UNCOND_BR)
                    return false;

                return instruction.relative_to_absolute() == target_address;
                };
        }

    }
}

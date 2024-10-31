#include "../zydis_wrapper.hpp"   

namespace zydis {
    namespace encoder {

        encoded_instructions encode(const ZydisEncoderRequest& request) {
            std::vector<uint8_t> final_bytes{};

            uint8_t encoded[ZYDIS_MAX_INSTRUCTION_LENGTH];
            size_t encoded_length = sizeof(encoded);

            if (ZYAN_SUCCESS(ZydisEncoderEncodeInstruction(&request, encoded, &encoded_length))) {
                final_bytes.insert(final_bytes.end(), encoded, encoded + encoded_length);
            }

            return encoded_instructions(final_bytes);
        }

        ZydisInstructionAttributes get_segment_prefix(ZydisRegister reg) {
            switch (reg) {
            case ZYDIS_REGISTER_GS: return ZYDIS_ATTRIB_HAS_SEGMENT_GS;
            case ZYDIS_REGISTER_FS: return ZYDIS_ATTRIB_HAS_SEGMENT_FS;
            case ZYDIS_REGISTER_ES: return ZYDIS_ATTRIB_HAS_SEGMENT_ES;
            case ZYDIS_REGISTER_DS: return ZYDIS_ATTRIB_HAS_SEGMENT_DS;
            case ZYDIS_REGISTER_SS: return ZYDIS_ATTRIB_HAS_SEGMENT_SS;
            case ZYDIS_REGISTER_CS: return ZYDIS_ATTRIB_HAS_SEGMENT_CS;
            default: return 0;
            }
        }

        encoded_instructions encode(const std::vector<std::pair<ZydisMnemonic, std::vector<operand>>>& instructions) {
            std::vector<uint8_t> final_bytes{};

            for (const auto& [mnemonic, operands] : instructions) {
                ZydisEncoderRequest request{};
                request.mnemonic = mnemonic;
                request.machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
                request.operand_count = operands.size();

                for (size_t i = 0; i < operands.size(); ++i) {
                    if (i >= ZYDIS_ENCODER_MAX_OPERANDS)
                        continue;

                    const operand& op = operands[i];
                    ZydisEncoderOperand& req_operand = request.operands[i];

                    std::visit([&req_operand, &request](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, register_operand>) {
                            req_operand.type = ZYDIS_OPERAND_TYPE_REGISTER;
                            req_operand.reg.value = arg;
                        }
                        else if constexpr (std::is_same_v<T, memory_operand>) {
                            req_operand.type = ZYDIS_OPERAND_TYPE_MEMORY;
                            req_operand.mem.base = std::get<0>(arg);

                            auto prefix = get_segment_prefix(req_operand.mem.base);
                            if (prefix) {
                                request.prefixes = prefix;
                                req_operand.mem.base = ZYDIS_REGISTER_NONE;
                            }

                            req_operand.mem.index = std::get<1>(arg);
                            req_operand.mem.scale = std::get<2>(arg);
                            req_operand.mem.displacement = std::get<3>(arg);
                            req_operand.mem.size = std::get<4>(arg);
                        }
                        else if constexpr (std::is_same_v<T, pointer_operand>) {
                            req_operand.type = ZYDIS_OPERAND_TYPE_POINTER;
                            req_operand.ptr.segment = std::get<0>(arg);
                            req_operand.ptr.offset = std::get<1>(arg);
                        }
                        else if constexpr (std::is_same_v<T, immediate_operand>) {
                            req_operand.type = ZYDIS_OPERAND_TYPE_IMMEDIATE;
                            req_operand.imm.u = arg;
                        }
                        }, op);
                }

                uint8_t encoded[ZYDIS_MAX_INSTRUCTION_LENGTH];
                size_t encoded_length = sizeof(encoded);

                if (ZYAN_SUCCESS(ZydisEncoderEncodeInstruction(&request, encoded, &encoded_length))) {
                    final_bytes.insert(final_bytes.end(), encoded, encoded + encoded_length);
                }
            }

            return encoded_instructions(final_bytes);
        }

        encoded_instructions encode_absolute_jump(uintptr_t target) {
            auto encoded = encode
            (
                {

                { ZYDIS_MNEMONIC_JMP, { memory_operand(ZYDIS_REGISTER_RIP, ZYDIS_REGISTER_NONE, 0, 0, sizeof(uint64_t)) } }

                }
            );

            auto encoded_bytes = encoded.bytes();

            for (size_t i = 0; i < sizeof(target); ++i)
                encoded_bytes.push_back(static_cast<uint8_t>((target >> (i * 8)) & 0xFF));

            return encoded_instructions(encoded_bytes);
        }

        encoded_instructions encode_absolute_call(uintptr_t target) {
            auto encoded = encode
            (
                {

                { ZYDIS_MNEMONIC_CALL, { memory_operand(ZYDIS_REGISTER_RIP, ZYDIS_REGISTER_NONE, 0, 2, sizeof(uint64_t)) } },
                { ZYDIS_MNEMONIC_JMP, { immediate_operand(sizeof(immediate_operand)) } }
                
                }
            );

            auto encoded_bytes = encoded.bytes();

            for (size_t i = 0; i < sizeof(target); ++i)
                encoded_bytes.push_back(static_cast<uint8_t>((target >> (i * 8)) & 0xFF));

            return encoded_instructions(encoded_bytes);
        }

    }
}

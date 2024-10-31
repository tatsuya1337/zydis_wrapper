#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <windows.h>
#include <functional>

namespace zydis {

    // This isn't really needed, but without it IntelliSense will freak out and complain that instructions_iterator cannot access set_iteration_count
    class instructions_iterator;

    class single_instruction {
    public:
        explicit single_instruction();
        explicit single_instruction(const ZydisDisassembledInstruction& disassembled_instruction, size_t* skip_until = nullptr);
       
        template <typename T>
        explicit single_instruction(T addr) requires ((std::is_integral_v<T> && sizeof(T) == sizeof(uintptr_t)) || std::is_pointer_v<T>)
        {
            using CastType = std::conditional_t<std::is_pointer_v<T>, const void*, uintptr_t>;
            
            ZyanStatus status = ZydisDisassembleIntel(
                ZYDIS_MACHINE_MODE_LONG_64,
                reinterpret_cast<uintptr_t>(reinterpret_cast<const void*>(static_cast<CastType>(addr))),
                reinterpret_cast<const void*>(static_cast<CastType>(addr)),
                ZYDIS_MAX_INSTRUCTION_LENGTH,
                &disassembled_
            );

            if (!ZYAN_SUCCESS(status)) {
                disassembled_ = {};
                iteration_count_ = 0;
            }
            else {
                iteration_count_ = 0;
            }
        }

        uintptr_t address() const noexcept;
        const ZydisDecodedInstruction info() const noexcept;
        std::string text() const noexcept;
        bool is_valid() const noexcept;

        const ZydisDecodedOperand operand(int i) const noexcept;
        const ZydisEncoderRequest encoder_struct() const;
        uintptr_t relative_to_absolute() const;
        single_instruction next() const;
        single_instruction find(const InstructionCondition& condition) const noexcept;
        void write_to_raw(void* buffer) const noexcept;
        void write_to(void* buffer) const noexcept;
        void rewrite(const std::vector<uint8_t>& bytes) const;
        size_t iteration_count() const noexcept;

    private:
        void set_iteration_count(size_t count) noexcept;
        ZydisDisassembledInstruction disassembled_{};
        size_t iteration_count_ = 0;
        size_t* skip_until_ = nullptr;
        friend class instructions_iterator;
    };

}

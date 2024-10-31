#include "../zydis_wrapper.hpp"

namespace zydis {


    instructions_iterator::instructions_iterator(uintptr_t address)
        : address_(address), status_(ZYAN_STATUS_SUCCESS), count_(0), skip_until_(0) {
        decode();
    }

    const single_instruction& instructions_iterator::operator*() const noexcept {
        return instruction_;
    }

    const single_instruction* instructions_iterator::operator->() const noexcept {
        return &instruction_;
    }

    instructions_iterator& instructions_iterator::operator++() {
        if (ZYAN_SUCCESS(status_)) {
            address_ += instruction_.info().length;
            ++count_;
            decode();
        }
        return *this;
    }

    instructions_iterator instructions_iterator::operator++(int) {
        instructions_iterator temp = *this;
        ++(*this);
        return temp;
    }

    bool instructions_iterator::operator==(const instructions_iterator& other) const noexcept {
        if (!ZYAN_SUCCESS(status_) && !ZYAN_SUCCESS(other.status_))
            return true;

        return address_ == other.address_;
    }

    bool instructions_iterator::operator!=(const instructions_iterator& other) const noexcept {
        return !(*this == other);
    }

    void instructions_iterator::decode() {
        if (address_ < skip_until_) {
            address_ = skip_until_;
        }

        ZydisDisassembledInstruction disassembled_instruction{};
        const ZyanU8* data = reinterpret_cast<const ZyanU8*>(address_);
        status_ = ZydisDisassembleIntel(
            ZYDIS_MACHINE_MODE_LONG_64,
            address_,
            data,
            ZYDIS_MAX_INSTRUCTION_LENGTH,
            &disassembled_instruction
        );

        if (ZYAN_SUCCESS(status_)) {
            instruction_ = single_instruction(disassembled_instruction, &skip_until_);
            instruction_.set_iteration_count(count_);
        }
        else {
            instruction_ = single_instruction();
        }
    }

    instructions_iterator instructions::begin() const {
        return instructions_iterator(start_address_);
    }

    instructions_iterator instructions::end() const {
        return instructions_iterator();
    }

}

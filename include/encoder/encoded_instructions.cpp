#include "../zydis_wrapper.hpp"

namespace zydis {
    namespace encoder {

        encoded_instructions::encoded_instructions() = default;

        encoded_instructions::encoded_instructions(const std::vector<uint8_t>& bytes)
            : bytes_(bytes) {}

        const std::vector<uint8_t>& encoded_instructions::bytes() const noexcept {
            return bytes_;
        }

        std::size_t encoded_instructions::size() const noexcept {
            return bytes_.size();
        }

        bool encoded_instructions::is_valid() const noexcept {
            return !bytes_.empty();
        }

        void encoded_instructions::write_to_raw(void* buffer) const noexcept {
            if (!is_valid() || !buffer) return;
            std::memcpy(buffer, bytes_.data(), bytes_.size());
        }

        void encoded_instructions::write_to(void* buffer) const noexcept {
            if (!is_valid() || !buffer) return;

            DWORD oldProtect = 0;
            VirtualProtect(buffer, bytes_.size(), PAGE_EXECUTE_READWRITE, &oldProtect);

            write_to_raw(buffer);

            VirtualProtect(buffer, bytes_.size(), oldProtect, &oldProtect);
        }

        std::vector<single_instruction> encoded_instructions::decode() const {
            if (!is_valid()) return {};

            std::vector<single_instruction> instructions_array{};
            uintptr_t start_address = reinterpret_cast<uintptr_t>(bytes_.data());
            uintptr_t end_address = start_address + bytes_.size();

            for (const auto& instruction : zydis::instructions(start_address)) {
                if (instruction.address() >= end_address) break;
                instructions_array.emplace_back(instruction);
            }

            return instructions_array;
        }

        encoded_instructions& encoded_instructions::operator+=(const encoded_instructions& other) {
            bytes_.insert(bytes_.end(), other.bytes_.begin(), other.bytes_.end());
            return *this;
        }

        encoded_instructions operator+(encoded_instructions lhs, const encoded_instructions& rhs) {
            lhs += rhs;
            return lhs;
        }

    }
}

#pragma once

#include <vector>
#include <cstdint>
#include <cstring>
#include <windows.h>
#include "../instruction/single_instruction.hpp"
#include "../instruction/instructions.hpp"

namespace zydis {
    namespace encoder {

        class encoded_instructions {
        public:
            explicit encoded_instructions();
            explicit encoded_instructions(const std::vector<uint8_t>& bytes);

            const std::vector<uint8_t>& bytes() const noexcept;
            std::size_t size() const noexcept;
            bool is_valid() const noexcept;

            void write_to_raw(void* buffer) const noexcept;
            void write_to(void* buffer) const noexcept;

            std::vector<single_instruction> decode() const;

            encoded_instructions& operator+=(const encoded_instructions& other);
            friend encoded_instructions operator+(encoded_instructions lhs, const encoded_instructions& rhs);

        private:
            std::vector<uint8_t> bytes_;
        };

    }
}

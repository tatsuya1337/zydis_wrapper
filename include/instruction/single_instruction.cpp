#include "../zydis_wrapper.hpp"

#include <iostream>


namespace zydis {

	single_instruction::single_instruction() = default;

	single_instruction::single_instruction(const ZydisDisassembledInstruction& disassembled_instruction, size_t* skip_until)
		: disassembled_(disassembled_instruction), iteration_count_(0) {}

	uintptr_t single_instruction::address() const noexcept {
		return disassembled_.runtime_address;
	}

	const ZydisDecodedInstruction single_instruction::info() const noexcept {
		return disassembled_.info;
	}

	std::string single_instruction::text() const noexcept {
		return disassembled_.text;
	}

	bool single_instruction::is_valid() const noexcept {
		return address() != 0;
	}

	const ZydisDecodedOperand single_instruction::operand(int i) const noexcept {
		if (i >= 0 && i < info().operand_count)
			return disassembled_.operands[i];

		return ZydisDecodedOperand{};
	}

	const ZydisEncoderRequest single_instruction::encoder_struct() const {
		ZydisEncoderRequest request{};

		ZyanStatus status = ZydisEncoderDecodedInstructionToEncoderRequest(
			&disassembled_.info,
			disassembled_.operands,
			info().operand_count_visible,
			&request
		);

		if (ZYAN_SUCCESS(status))
			return request;

		return {};
	}

	uintptr_t single_instruction::relative_to_absolute() const {
		if (!is_valid())
			return 0;

		int operand_index = 0;

		for (uint8_t i = 0; i < info().operand_count; ++i) {
			ZydisDecodedOperand op = operand(i);
			if (op.type == ZYDIS_OPERAND_TYPE_MEMORY) {
				if (op.mem.base == ZYDIS_REGISTER_RIP) {
					operand_index = i;
					break;
				}
			}
		}

		uintptr_t absolute_addr = 0;
		ZyanStatus status = ZydisCalcAbsoluteAddress(&disassembled_.info, &disassembled_.operands[operand_index], address(), &absolute_addr);

		if (ZYAN_SUCCESS(status))
			return absolute_addr;

		return 0;
	}

	single_instruction single_instruction::next() const {
		if (!is_valid())
			return single_instruction();

		uintptr_t next_address = address() + disassembled_.info.length;

		if (skip_until_ && next_address < *skip_until_) {
			next_address = *skip_until_;
		}

		ZydisDisassembledInstruction next_instruction{};
		ZyanStatus status = ZydisDisassembleIntel(
			ZYDIS_MACHINE_MODE_LONG_64,
			next_address,
			reinterpret_cast<const void*>(next_address),
			ZYDIS_MAX_INSTRUCTION_LENGTH,
			&next_instruction
		);

		if (ZYAN_SUCCESS(status))
			return single_instruction(next_instruction, skip_until_);

		return single_instruction();
	}

	single_instruction single_instruction::find(const InstructionCondition& condition) const noexcept {
		single_instruction current = *this;

		while (current.is_valid() && current.info().mnemonic != ZYDIS_MNEMONIC_INT3) {
			if (condition(current)) {
				return current;
			}
			current = current.next();
		}

		return single_instruction();
	}

	void single_instruction::write_to_raw(void* buffer) const noexcept {
		if (!is_valid())
			return;

		if (buffer)
			std::memcpy(buffer, reinterpret_cast<void*>(address()), info().length);
	}

	void single_instruction::write_to(void* buffer) const noexcept {
		if (!is_valid())
			return;

		if (buffer) {
			DWORD oldProtect = 0;
			VirtualProtect((LPVOID)buffer, info().length, PAGE_EXECUTE_READWRITE, &oldProtect);

			write_to_raw(buffer);

			VirtualProtect((LPVOID)buffer, info().length, oldProtect, &oldProtect);
		}
	}

	void single_instruction::rewrite(const std::vector<uint8_t>& bytes) const {
		if (!is_valid() || bytes.empty()) return;

		DWORD oldProtect = 0;
		VirtualProtect(reinterpret_cast<LPVOID>(address()), bytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect);

		std::memcpy(reinterpret_cast<void*>(address()), bytes.data(), bytes.size());

		VirtualProtect(reinterpret_cast<LPVOID>(address()), bytes.size(), oldProtect, &oldProtect);

		single_instruction instr = *this;

		while (instr.next().address() < address() + bytes.size()) {
			instr = instr.next();
		}

		if (skip_until_) {
			*skip_until_ = instr.next().address();
		}
	}

	void single_instruction::set_iteration_count(size_t count) noexcept {
		iteration_count_ = count;
	}

	size_t single_instruction::iteration_count() const noexcept {
		return iteration_count_;
	}

}

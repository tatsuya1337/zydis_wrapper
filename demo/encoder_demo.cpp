#include "encoder_demo.hpp"

namespace zydis_wrapper_demo {

	void encoder()
	{
		auto current_process_id = zydis::encoder::encode
		(
			{

			{ ZYDIS_MNEMONIC_MOV, { zydis::encoder::register_operand(ZYDIS_REGISTER_RAX), zydis::encoder::memory_operand(ZYDIS_REGISTER_GS, ZYDIS_REGISTER_NONE, 0, 0x30, sizeof(uint64_t)) } },
			{ ZYDIS_MNEMONIC_MOV, { zydis::encoder::register_operand(ZYDIS_REGISTER_EAX), zydis::encoder::memory_operand(ZYDIS_REGISTER_RAX, ZYDIS_REGISTER_NONE, 0, 0x40, sizeof(uint32_t)) } },
			{ ZYDIS_MNEMONIC_RET, {  } }

			}
		);

		auto current_process_id_buffer = VirtualAlloc(nullptr, current_process_id.size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		current_process_id.write_to(current_process_id_buffer);

		std::cout << std::hex << "current_process_id_buffer: " << current_process_id_buffer << std::endl;
		std::cout << std::dec << "Crafted GetCurrentProcessId: " << reinterpret_cast<decltype(&GetCurrentProcessId)>(current_process_id_buffer) << std::endl;
		std::cout << std::dec << "Original GetCurrentProcessId: " << GetCurrentProcessId() << std::endl;

		auto random_instructions = zydis::encoder::encode
		(
			{

			{ ZYDIS_MNEMONIC_MOV, { zydis::encoder::register_operand(ZYDIS_REGISTER_RAX), zydis::encoder::register_operand(ZYDIS_REGISTER_RCX) } },
			{ ZYDIS_MNEMONIC_ADD, { zydis::encoder::register_operand(ZYDIS_REGISTER_RAX), zydis::encoder::immediate_operand(0x1234) } },
			{ ZYDIS_MNEMONIC_XOR, { zydis::encoder::register_operand(ZYDIS_REGISTER_R8), zydis::encoder::register_operand(ZYDIS_REGISTER_R8) } },

			}
		);


		for (const auto& encoded_instruction : random_instructions.decode())
			std::cout << "random_instructions text: " << encoded_instruction.text() << std::endl;

		static const wchar_t* newtext = L"This message is modified dynamically through manually crafted shellcode.";

		auto messagebox_stub = zydis::encoder::encode
		(
			{

			{ ZYDIS_MNEMONIC_NOP, { } },
			{ ZYDIS_MNEMONIC_MOV, { zydis::encoder::register_operand(ZYDIS_REGISTER_RDX), zydis::encoder::immediate_operand(newtext) } },

			}
		);

		messagebox_stub += zydis::encoder::encode_absolute_jump((uintptr_t)MessageBoxW);

		auto messageboxw_buffer = VirtualAlloc(nullptr, messagebox_stub.size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		messagebox_stub.write_to(messageboxw_buffer);

		reinterpret_cast<decltype(&MessageBoxW)>(messageboxw_buffer)(NULL, L"I am the original, and no one will ever see me. Unless you comment out ZYDIS_MNEMONIC_MOV operand above.", L"Error", MB_ICONINFORMATION);
	}
}
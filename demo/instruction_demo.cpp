#include "instruction_demo.hpp"

namespace zydis_wrapper_demo {

	uintptr_t sample_value = 0xdead;

#pragma optimize( "", off )
	void sample_function()
	{
		if (sample_value == 0xdead)
			MessageBoxW(NULL, L"I'm dead.", L"Error", MB_ICONERROR);
		else if (sample_value == 0xbeef)
			MessageBoxW(NULL, L"I'm beef.", L"Success", MB_ICONINFORMATION);
	}
#pragma optimize( "", on )

	void sample_function_find()
	{
		// Searches an instruction from given zydis::single_instruction with zydis::InstructionCondition function.
		// There are some template functions in helper/find_predicates.hpp

		// You can also make your own check function using FIND_SIGNATURE_LAMBDA macro, and pass it to find function to suit your need.
		// In the function you can access the zydis::single_instruction object that is being evaluated by accessing the instruction variable.

		// Checks if it is CMP instruction and it is comparing the value 0xdead
		auto is_cmp = FIND_SIGNATURE_LAMBDA
		{ return instruction.info().mnemonic == ZYDIS_MNEMONIC_CMP && instruction.operand(1).imm.value.u == 0xdead; };

		auto instruction = zydis::single_instruction(sample_function);

		auto cmp_instruction = instruction.find(is_cmp);
		auto epilogue_instruction = instruction.find(zydis::find_predicates::is_epilogue);
		auto first_messageboxw_instruction = instruction.find(zydis::find_predicates::is_call_to((uintptr_t)MessageBoxW));

		std::cout << "cmp_instruction: " << std::hex << cmp_instruction.address() << std::endl;
		std::cout << "epilogue_instruction: " << std::hex << epilogue_instruction.address() << std::endl;
		std::cout << "first_messageboxw_instruction: " << std::hex << first_messageboxw_instruction.address() << std::endl;
	}

	void sample_function_modification_copy()
	{
		auto buffer = malloc(0x1000);
		uint32_t offset = 0;

		std::cout << std::hex << "buffer: " << buffer << std::endl;

		for (const auto& instruction : zydis::instructions(sample_function))
		{
			// Copies the instruction to the destination
			instruction.write_to_raw((void*)((uintptr_t)(buffer)+offset));

			// Instruction size
			offset += instruction.info().length;

			// In practical usage, you shouldn't rely on only RET to determine the end of the function
			if (instruction.info().mnemonic == ZYDIS_MNEMONIC_RET)
				break;
		}

		for (const auto& instruction : zydis::instructions(buffer))
		{
			std::cout << std::hex << "Copied instruction: " << instruction.text() << std::endl;

			if (instruction.info().mnemonic == ZYDIS_MNEMONIC_RET)
				break;
		}

	}

	void sample_function_modification_1()
	{
		sample_function();

		std::cout << std::hex << "sample_function: " << sample_function << std::endl;

		// The iteration will keep going until zydis fails to decode the instruction
		for (const auto& instruction : zydis::instructions(sample_function))
		{
			if (instruction.operand(1).imm.value.u == 0xdead)
			{
				std::cout << "possible sample_value instruction: " << instruction.text() << std::endl;
				std::cout << "address: " << std::hex << instruction.relative_to_absolute() << std::endl;
				std::cout << "iteration count aka instruction count: " << std::dec << instruction.iteration_count() << std::endl;
				std::cout << "next instruction address: " << std::hex << instruction.next().address() << std::endl;

				*(uintptr_t*)instruction.relative_to_absolute() = 0xbeef;
			}

			// In practical usage, you shouldn't rely on only RET to determine the end of the function
			if (instruction.info().mnemonic == ZYDIS_MNEMONIC_RET)
				break;
		}

		sample_function();
	}

	void sample_function_modification_2()
	{
		sample_function();

		std::cout << std::hex << "sample_function: " << sample_function << std::endl;

		auto is_branch = FIND_SIGNATURE_LAMBDA
		{ return instruction.info().meta.category == ZYDIS_CATEGORY_COND_BR || instruction.info().meta.category == ZYDIS_CATEGORY_UNCOND_BR; };

		auto is_target = FIND_SIGNATURE_LAMBDA
		{ return instruction.operand(0).reg.value == ZYDIS_REGISTER_R9D && instruction.operand(1).imm.value.u == MB_ICONINFORMATION; };

		auto instruction = zydis::single_instruction(sample_function);

		// First found branch instruction
		auto first_branch_instruction = instruction.find(is_branch);

		// Gets the target instruction by looking for unique value, for this example I use MB_ICONINFORMATION
		auto target_instruction = instruction.find(is_target);

		// Encodes a jump instruction with an absolute address without using any register other than RIP 
		auto absolute_jump_to_target = zydis::encoder::encode_absolute_jump(target_instruction.address());

		first_branch_instruction.rewrite(absolute_jump_to_target.bytes());

		sample_function();
	}

	void sample_function_modification_3()
	{
		sample_function();

		std::cout << std::hex << "sample_function: " << sample_function << std::endl;

		auto is_first_cmp = FIND_SIGNATURE_LAMBDA
		{ return instruction.info().mnemonic == ZYDIS_MNEMONIC_CMP && instruction.operand(1).imm.value.u == 0xdead; };

		auto is_second_cmp = FIND_SIGNATURE_LAMBDA
		{ return instruction.info().mnemonic == ZYDIS_MNEMONIC_CMP && instruction.operand(1).imm.value.u == 0xbeef; };

		auto instruction = zydis::single_instruction(sample_function);

		auto first_cmp_instruction = instruction.find(is_first_cmp);
		auto second_cmp_instruction = instruction.find(is_second_cmp);

		auto first_cmp_struct = first_cmp_instruction.encoder_struct();
		auto second_cmp_struct = second_cmp_instruction.encoder_struct();

		first_cmp_struct.operands[1].imm.u = 0xfade;
		second_cmp_struct.operands[1].imm.u = 0xdead;

		first_cmp_instruction.rewrite(zydis::encoder::encode(first_cmp_struct).bytes());
		second_cmp_instruction.rewrite(zydis::encoder::encode(second_cmp_struct).bytes());

		sample_function();
	}

	void instruction()
	{
		//zydis::single_instruction::find demonstration.
		//sample_function_find();

		// Loops through all instructions in sample_function until it reaches RET and then copy them into malloc buffer.
		// Please note that the copied function cannot be called unless relocation is fixed.
		//sample_function_modification_copy();

	
		// Below are examples to show how you can possibly modify a function using zydis_wrapper.
		// They will apply different modifications to sample_function() so the second if will get executed.
		
		// When you comment out those samples make sure only one of them gets executed on runtime or the process may crash.
		

		// Gets the address of sample_value variable and set the value to 0xbeef directly.
		//sample_function_modification_1();

		// To avoid properly locating the prologue section, it gets the first branch instruction and then places a jump instruction there to the 0xbeef bracket directly.
		//sample_function_modification_2();

		// Changes if instructions(cmp instructions) so they match with the default value of sample_value without touching the variable.
		sample_function_modification_3();
	}
}
# zydis_wrapper

**zydis_wrapper** is a modern C++ wrapper for [Zydis](https://github.com/zyantific/zydis), offering a user-friendly interface for disassembling and encoding Windows x64 Intel machine code. It simplifies the process by providing intuitive functions and abstractions, making it easier to work with low-level code.

![Platform](https://img.shields.io/badge/platform-Windows-blue)
![C++ Version](https://img.shields.io/badge/C%2B%2B-17%2B-brightgreen)

## Features

- **Modern C++ Interface**: Utilizes C++17 features for clean and efficient code.
- **Dynamic Encoding**: Supports runtime encoding of instructions.
- **Intuitive Iteration**: Provides easy-to-use iterators for traversing instructions.
- **Utility Functions**: Allows you to access various elements of instructions with ease.

## Examples

The following examples demonstrate how to use the `zydis_wrapper` for encoding and decoding instructions, iterating over them, and resolving relative addresses.

### Dynamic syscall shellcode

```cpp
uint32_t syscall_index = 0x3F;

auto NtRVM = zydis::encoder::encode({
    {
        ZYDIS_MNEMONIC_MOV,
        {
            zydis::encoder::register_operand(ZYDIS_REGISTER_R10),
            zydis::encoder::register_operand(ZYDIS_REGISTER_RCX)
        }
    },
    {
        ZYDIS_MNEMONIC_MOV,
        {
            zydis::encoder::register_operand(ZYDIS_REGISTER_EAX),
            zydis::encoder::immediate_operand(syscall_index)
        }
    },
    { ZYDIS_MNEMONIC_SYSCALL, {} },
    { ZYDIS_MNEMONIC_RET, {} }
});

```

This encodes to:

```asm
mov r10, rcx
mov eax, 0x3F
syscall
ret
```

You can also decode and print the instructions as text.

```cpp
 for (const auto& decoded_instruction : NtRVM.decode())
     std::cout << "NtRVM text: " << decoded_instruction.text() << std::endl;
```

### Iteration and resolving rip-relative instructions

```asm
; asm_function
48 8B 0D 77 6E F1 A0 - mov rcx, [0x7FF7EF5923B4]
E8 B0 E2 FF FF - call 0x7FF69DFD7BD0
```
You can pass an integer or a pointer to zydis::instructions.
The iteration only stops when Zydis fails to decode an instruction, therefore it's recommended to insert your own condition to break the loop when necessary.

```cpp
for (const auto& instruction : zydis::instructions(asm_function))
{
    if (instruction.info().mnemonic == ZYDIS_MNEMONIC_MOV)
    {
        auto absolute_address = instruction.relative_to_absolute();
    
        // absolute_address should be 0x7FF7EF5923B4
        std::cout << std::hex << "Mov absolute address: " << absolute_address << std::endl;
    }

    if (instruction.info().mnemonic == ZYDIS_MNEMONIC_CALL)
    {
        auto absolute_address = instruction.relative_to_absolute();

        // absolute_address should be 0x7FF69DFD7BD0
        std::cout << std::hex << "Call absolute address: " << absolute_address << std::endl;
    }

    if (instruction.info().mnemonic == ZYDIS_MNEMONIC_RET)
        break;
}
```

Or alternatively, if you already know the instruction addresses you can construct zydis::single_instruction objects directly:

```cpp
auto mov_instruction = zydis::single_instruction(0x7FF7EF5923CA);
auto call_instruction = zydis::single_instruction(0x7FF7EF5923D1);
```

it also accepts an integer or a pointer as zydis::instructions does.

### More examples

#### [encoder_demo.cpp](./demo/encoder_demo.cpp)

Demonstrates how to encode machine code at runtime.

#### [instruction_demo.cpp](./demo/instruction_demo.cpp)

Focuses on iterating over instructions and interacting with them.

## Prerequisites

- **C++17** or higher
- [Zydis Library](https://github.com/zyantific/zydis)
- **Visual Studio 2022** (if you intend to use the `.sln` file provided in this repository)

## Installation

1. **Add the Wrapper Files:**
   - Copy all the files from the `include` directory into your project's include path.

2. **Include the Header:**
   - In your source files, include the wrapper header:

     ```cpp
     #include "zydis_wrapper.hpp"
     ```

   Ensure that the path to `zydis_wrapper.hpp` matches your project's directory structure.

3. **Configure Zydis Path**
   - Make sure Zydis can be found in one of these paths:

     ```cpp
     #include <Zydis/Zydis.h>
     #include "Zydis/Zydis.h"
     #include "Zydis.h"
     ```

     `zydis_wrapper.hpp` will try to include Zydis from one of them using [#if __has_include](https://en.cppreference.com/w/cpp/preprocessor/include) macro.
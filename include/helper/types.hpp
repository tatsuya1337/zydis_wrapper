#pragma once
#define FIND_SIGNATURE_LAMBDA [&](const zydis::single_instruction& instruction) -> bool

#include <functional>

namespace zydis {

    class single_instruction;
    using InstructionCondition = std::function<bool(const single_instruction&)>;
}

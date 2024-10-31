#pragma once

#if __has_include(<Zydis/Zydis.h>)
#include <Zydis/Zydis.h>
#elif __has_include("Zydis/Zydis.h")
#include "Zydis/Zydis.h"
#elif __has_include("Zydis.h")
#include "Zydis.h"
#else
#error "Zydis not found"
#endif

#include "helper/types.hpp"
#include "helper/find_predicates.hpp"

#include "instruction/single_instruction.hpp"
#include "instruction/instructions.hpp"

#include "encoder/encoded_instructions.hpp"
#include "encoder/encoder.hpp"


#include "demo/instruction_demo.hpp"
#include "demo/encoder_demo.hpp"

int main()
{
    const std::string blue = "\033[34m";
    const std::string green = "\033[32m";
    const std::string reset = "\033[0m";

    std::cout << blue << "Instruction demo begins" << reset << std::endl;
    zydis_wrapper_demo::instruction();

    std::cout << std::endl;

    std::cout << green << "Encoder demo begins" << reset << std::endl;
    zydis_wrapper_demo::encoder();

    getchar();
    return 0;
}
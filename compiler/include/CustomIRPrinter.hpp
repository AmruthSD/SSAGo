#pragma once

#include <CustomIR.hpp>
#include <iostream>

namespace custom_ir {
class IRPrinter {
public:
  static void print(ModuleIR *module);

private:
  static void printFunction(Function *func);
  static void printBasicBlock(BasicBlockIR *bb);
  static void printInstruction(Instruction *inst);

  static std::string opcodeToString(Opcode op);
};
} // namespace custom_ir

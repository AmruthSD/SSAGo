#pragma once

#include <CustomIR.hpp>
#include <fstream>
#include <iostream>

namespace custom_ir {
class IRPrinter {
public:
  static void print(ModuleIR *module, const std::string &filename);

private:
  std::ostream *out = &std::cout;
  static void printFunction(Function *func);
  static void printBasicBlock(BasicBlockIR *bb);
  static void printInstruction(Instruction *inst);
  static void printValue(Value *val);

  static std::string opcodeToString(Opcode op);
};
} // namespace custom_ir

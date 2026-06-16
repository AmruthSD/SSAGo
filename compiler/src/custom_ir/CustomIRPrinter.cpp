#include <CustomIR.hpp>
#include <CustomIRPrinter.hpp>

namespace custom_ir {

void IRPrinter::print(ModuleIR *module) {
  for (auto &[name, func] : module->functions) {
    printFunction(func);
    std::cout << "\n";
  }
}

void IRPrinter::printFunction(Function *func) {
  std::cout << "function " << func->name << "\n";
  std::cout << "{\n";

  for (auto *block : func->blocks) {
    printBasicBlock(block);
  }

  std::cout << "}\n";
}

void IRPrinter::printBasicBlock(BasicBlockIR *bb) {
  std::cout << "  " << bb->name << ":\n";

  if (!bb->predecessor.empty()) {
    std::cout << "    preds: ";
    for (auto *pred : bb->predecessor)
      std::cout << pred->name << " ";
    std::cout << "\n";
  }

  if (!bb->successor.empty()) {
    std::cout << "    succs: ";
    for (auto *succ : bb->successor)
      std::cout << succ->name << " ";
    std::cout << "\n";
  }

  for (auto *inst : bb->instructions) {
    std::cout << "    ";
    printInstruction(inst);
    std::cout << "\n";
  }

  std::cout << "\n";
}

void IRPrinter::printInstruction(Instruction *inst) {
  std::cout << inst->value << " = " << opcodeToString(inst->opcode);

  if (!inst->operands.empty()) {
    std::cout << " ";

    for (size_t i = 0; i < inst->operands.size(); i++) {
      std::cout << inst->operands[i]->value;

      if (i + 1 < inst->operands.size())
        std::cout << ", ";
    }
  }
}

std::string IRPrinter::opcodeToString(Opcode op) {
  switch (op) {
  case Opcode::Add:
    return "add";
  case Opcode::Sub:
    return "sub";
  case Opcode::Mul:
    return "mul";
  case Opcode::Div:
    return "div";

  case Opcode::FAdd:
    return "fadd";
  case Opcode::FSub:
    return "fsub";
  case Opcode::FMul:
    return "fmul";
  case Opcode::FDiv:
    return "fdiv";

  case Opcode::ICmpEQ:
    return "icmp_eq";
  case Opcode::ICmpNE:
    return "icmp_ne";
  case Opcode::ICmpLT:
    return "icmp_lt";
  case Opcode::ICmpGT:
    return "icmp_gt";

  case Opcode::FCmpEQ:
    return "fcmp_eq";
  case Opcode::FCmpNE:
    return "fcmp_ne";
  case Opcode::FCmpLT:
    return "fcmp_lt";
  case Opcode::FCmpGT:
    return "fcmp_gt";

  case Opcode::And:
    return "and";
  case Opcode::Or:
    return "or";

  case Opcode::Load:
    return "load";
  case Opcode::Store:
    return "store";

  case Opcode::Jump:
    return "jump";
  case Opcode::CondBranch:
    return "condbr";
  case Opcode::Branch:
    return "branch";

  case Opcode::Call:
    return "call";
  case Opcode::CallExternal:
    return "call_external";
  case Opcode::GoCall:
    return "gocall";
  case Opcode::Return:
    return "ret";

  case Opcode::Cast_INT:
    return "cast_int";
  case Opcode::Cast_FLOAT:
    return "cast_float";
  case Opcode::Bit_Cast:
    return "bitcast";

  case Opcode::Global_Dec:
    return "global";
  case Opcode::Alloca:
    return "alloca";

  case Opcode::Phi:
    return "phi";
  }

  return "unknown";
}
} // namespace custom_ir
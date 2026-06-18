#include <LLVMIRGenerator.hpp>

namespace custom_ir {
llvm::Value *LLVMIRGenerator::generateInstruction(Instruction *inst) {
  Opcode op = inst->opcode;

  switch (op) {

  case Opcode::Load:
    return generateLoad(inst);
    break;

  case Opcode::Store:
    return generateStore(inst);
    break;

  case Opcode::CondBranch:
    return generateCondBranch(inst);
    break;

  case Opcode::Branch:
    return generateBranch(inst);
    break;

  case Opcode::GoCall:
    return generateGoFunc(inst);
    break;

  case Opcode::Call:
    return generateFunctionCall(inst);
    break;

  case Opcode::CallExternal:
    return generateExternalCall(inst);
    break;

  case Opcode::Return:
    return generateReturn(inst);
    break;

  case Opcode::Cast_INT:
  case Opcode::Cast_FLOAT:
    return generateCast(inst);
    break;

  case Opcode::Alloca:
    return generateAlloca(inst);
    break;

  default:
    throw std::runtime_error("error unknown operator");
    break;
  }

  return nullptr;
}

llvm::Value *
LLVMIRGenerator::generateBinaryInstruction(BinaryInstruction *inst) {
  std::cout << "Binary instruction generation" << std::endl;
  Opcode op = inst->opcode;
  Value *l = inst->operands[0], *r = inst->operands[1];
  llvm::Value *L = l->llvm_codegen(this), *R = r->llvm_codegen(this);
  Value *res = inst->operands[2];
  switch (op) {

  case Opcode::FAdd:
    return tempValues[res->value] = builder.CreateFAdd(L, R, res->value);

  case Opcode::FSub:
    return tempValues[res->value] = builder.CreateFSub(L, R, res->value);

  case Opcode::FMul:
    return tempValues[res->value] = builder.CreateFMul(L, R, res->value);

  case Opcode::FDiv:
    return tempValues[res->value] = builder.CreateFDiv(L, R, res->value);

  case Opcode::FCmpEQ:
    return tempValues[res->value] = builder.CreateFCmpOEQ(L, R, res->value);

  case Opcode::FCmpNE:
    return tempValues[res->value] = builder.CreateFCmpONE(L, R, res->value);

  case Opcode::FCmpLT:
    return tempValues[res->value] = builder.CreateFCmpOLT(L, R, res->value);

  case Opcode::FCmpGT:
    return tempValues[res->value] = builder.CreateFCmpOGT(L, R, res->value);

  case Opcode::Add:
    return tempValues[res->value] = builder.CreateAdd(L, R, res->value);

  case Opcode::Sub:
    return tempValues[res->value] = builder.CreateSub(L, R, res->value);

  case Opcode::Mul:
    return tempValues[res->value] = builder.CreateMul(L, R, res->value);

  case Opcode::Div:
    return tempValues[res->value] = builder.CreateSDiv(L, R, res->value);

  case Opcode::ICmpEQ:
    return tempValues[res->value] = builder.CreateICmpEQ(L, R, res->value);

  case Opcode::ICmpNE:
    return tempValues[res->value] = builder.CreateICmpNE(L, R, res->value);

  case Opcode::ICmpLT:
    return tempValues[res->value] = builder.CreateICmpSLT(L, R, res->value);

  case Opcode::ICmpGT:
    return tempValues[res->value] = builder.CreateICmpSGT(L, R, res->value);

  case Opcode::And:
    return tempValues[res->value] = builder.CreateAnd(L, R, res->value);

  case Opcode::Or:
    return tempValues[res->value] = builder.CreateOr(L, R, res->value);

  default:
    throw std::runtime_error("opcode not found for binary inst");
    return nullptr;
  }
}

llvm::Value *LLVMIRGenerator::generatePhiInstruction(PhiInstruction *inst) {}
} // namespace custom_ir
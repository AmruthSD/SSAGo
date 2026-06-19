#include <CustomIR.hpp>
#include <LLVMIRGenerator.hpp>

namespace custom_ir {
llvm::Value *Value::llvm_codegen(LLVMIRGenerator *gen) {
  throw std::runtime_error("no codegen for this " + this->value);
}

llvm::Value *ModuleIR::llvm_codegen(LLVMIRGenerator *gen) {
  return gen->generateModule(this);
}

llvm::Value *Constant::llvm_codegen(LLVMIRGenerator *gen) {
  return gen->generateLiteral(this);
}

llvm::Value *Function::llvm_codegen(LLVMIRGenerator *gen) {
  return gen->generateFunction(this);
}

llvm::Value *Instruction::llvm_codegen(LLVMIRGenerator *gen) {
  return gen->generateInstruction(this);
}

llvm::Value *BinaryInstruction::llvm_codegen(LLVMIRGenerator *gen) {
  return gen->generateBinaryInstruction(this);
}

llvm::Value *TempValue::llvm_codegen(LLVMIRGenerator *gen) {
  return gen->getTempValue(this);
}

llvm::Value *VariableValue::llvm_codegen(LLVMIRGenerator *gen) {
  return gen->getVariableValue(this);
}

llvm::Value *BasicBlockIR::llvm_codegen(LLVMIRGenerator *gen) {
  return gen->generateBlock(this);
}

llvm::Value *ConstantSizeof::llvm_codegen(LLVMIRGenerator *gen) {
  return gen->generateSizeofExpr(this);
}

llvm::Value *PhiInstruction::llvm_codegen(LLVMIRGenerator *gen) {
  return gen->generatePhiInstruction(this);
}
} // namespace custom_ir
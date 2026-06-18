#include <LLVMIRGenerator.hpp>

namespace custom_ir {

llvm::Value *LLVMIRGenerator::generateCondBranch(Instruction *inst) {
  BasicBlockIR *thenBB = dynamic_cast<BasicBlockIR *>(inst->operands[1]);
  BasicBlockIR *elseBB = dynamic_cast<BasicBlockIR *>(inst->operands[2]);
  llvm::Value *condVal = inst->operands[0]->llvm_codegen(this);

  return builder.CreateCondBr(condVal, getOrCreateBasicBlock(thenBB),
                              getOrCreateBasicBlock(elseBB));
}

llvm::Value *LLVMIRGenerator::generateBranch(Instruction *inst) {
  BasicBlockIR *thenBB = dynamic_cast<BasicBlockIR *>(inst->operands[0]);

  return builder.CreateBr(getOrCreateBasicBlock(thenBB));
}

llvm::Value *LLVMIRGenerator::generateReturn(Instruction *inst) {
  Value *val = (inst->operands[0]);
  llvm::Value *retVal = val->llvm_codegen(this);
  return builder.CreateRet(retVal);
}

} // namespace custom_ir
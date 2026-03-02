#include <IRGenerator.hpp>

llvm::Value *IRGenerator::generateIfElse(IfStmt *stmt) {

  llvm::Value *condValue = stmt->condition->codegen(*this);

  if (!condValue)
    throw std::runtime_error("Invalid condition in if");

  if (condValue->getType()->isIntegerTy()) {
    condValue = builder.CreateICmpNE(
        condValue, llvm::ConstantInt::get(condValue->getType(), 0), "ifcond");
  } else if (condValue->getType()->isFloatingPointTy()) {
    condValue = builder.CreateFCmpONE(
        condValue, llvm::ConstantFP::get(condValue->getType(), 0.0), "ifcond");
  } else if (!condValue->getType()->isIntegerTy(1)) {
    throw std::runtime_error("Unsupported condition type in if");
  }

  llvm::Function *function = builder.GetInsertBlock()->getParent();

  llvm::BasicBlock *thenBB =
      llvm::BasicBlock::Create(context, "then", function);
  llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(context, "else");
  llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(context, "ifcont");
  builder.CreateCondBr(condValue, thenBB, elseBB);

  builder.SetInsertPoint(thenBB);
  stmt->thenBranch->codegen(*this);
  if (!builder.GetInsertBlock()->getTerminator())
    builder.CreateBr(mergeBB);

  function->getBasicBlockList().push_back(elseBB);
  builder.SetInsertPoint(elseBB);
  if (stmt->elseBranch)
    stmt->elseBranch->codegen(*this);
  if (!builder.GetInsertBlock()->getTerminator())
    builder.CreateBr(mergeBB);
  function->getBasicBlockList().push_back(mergeBB);
  builder.SetInsertPoint(mergeBB);

  return nullptr;
}
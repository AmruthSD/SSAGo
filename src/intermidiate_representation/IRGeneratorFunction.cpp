#include <IRGenerator.hpp>
#include <llvm/IR/Verifier.h>

llvm::Function *IRGenerator::declareExternalFunction(
    const std::string &name, llvm::Type *returnType,
    std::vector<llvm::Type *> paramTypes, bool isVarArg) {
  llvm::Function *func = module->getFunction(name);
  if (func)
    return func;

  llvm::FunctionType *funcType =
      llvm::FunctionType::get(returnType, paramTypes, isVarArg);

  func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name,
                                module.get());

  return func;
}

llvm::Function *IRGenerator::getOrDeclarePrintf() {

  llvm::Type *i8PtrTy = llvm::Type::getInt8PtrTy(context);

  return declareExternalFunction("printf", llvm::Type::getInt32Ty(context),
                                 {i8PtrTy}, true);
}

llvm::Value *IRGenerator::generateFunction(FunctionStmt *func) {

  std::vector<llvm::Type *> paramTypes;
  for (auto &[name, type] : func->arguments)
    paramTypes.push_back(getLLVMType(type, context));

  llvm::FunctionType *functionType = llvm::FunctionType::get(
      getLLVMType(func->dataType, context), paramTypes, false);

  llvm::Function *function =
      llvm::Function::Create(functionType, llvm::Function::ExternalLinkage,
                             func->identifier, module.get());

  unsigned idx = 0;
  for (auto &arg : function->args())
    arg.setName(func->arguments[idx++].first);

  llvm::BasicBlock *entry =
      llvm::BasicBlock::Create(context, "entry", function);

  builder.SetInsertPoint(entry);

  namedValues.emplace_back();
  idx = 0;
  for (auto &arg : function->args()) {

    llvm::AllocaInst *alloca =
        builder.CreateAlloca(getLLVMType(func->arguments[idx].second, context),
                             nullptr, arg.getName());

    builder.CreateStore(&arg, alloca);
    namedValues.back()[arg.getName().str()] = alloca;

    idx++;
  }

  func->body.get()->codegen(*this);

  for (auto &block : *function) {
    if (!block.getTerminator()) {
      llvm::IRBuilder<> tmpBuilder(&block);

      if (function->getReturnType()->isVoidTy()) {
        tmpBuilder.CreateRetVoid();
      } else {
        tmpBuilder.CreateRet(
            llvm::Constant::getNullValue(function->getReturnType()));
      }
    }
  }

  namedValues.pop_back();

  builder.ClearInsertionPoint();
  return function;
}

llvm::Value *IRGenerator::generateBlock(BlockStmt *block) {
  if (namedValues.size() == 1)
    throw std::runtime_error("Unable to have block as global");
  namedValues.emplace_back();

  for (auto &stmt : block->body)
    stmt->codegen(*this);

  namedValues.pop_back();
  return nullptr;
}
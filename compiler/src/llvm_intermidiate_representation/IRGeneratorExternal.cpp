#include <CommonExternalFunctions.hpp>
#include <LLVMIRGenerator.hpp>

namespace custom_ir {
llvm::Function *LLVMIRGenerator::declareExternalFunction(
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

llvm::Function *LLVMIRGenerator::getOrDeclarePrintf() {

  llvm::Type *i8PtrTy = llvm::Type::getInt8PtrTy(context);

  return declareExternalFunction("printf", llvm::Type::getInt32Ty(context),
                                 {i8PtrTy}, true);
}

llvm::Function *LLVMIRGenerator::getOrDeclareScanf() {

  llvm::Type *i8PtrTy = llvm::Type::getInt8PtrTy(context);

  return declareExternalFunction("scanf", llvm::Type::getInt32Ty(context),
                                 {i8PtrTy}, true);
}

llvm::Function *LLVMIRGenerator::getOrDeclareMalloc() {
  llvm::Type *i8PtrTy = llvm::Type::getInt8PtrTy(context);
  llvm::Type *i64Ty = llvm::Type::getInt64Ty(context);

  return declareExternalFunction("malloc", i8PtrTy, {i64Ty}, false);
}

llvm::Function *LLVMIRGenerator::getOrDeclareSpawn() {

  llvm::Type *voidTy = llvm::Type::getVoidTy(context);
  llvm::Type *i8PtrTy = llvm::Type::getInt8PtrTy(context);

  return declareExternalFunction("runtime_spawn", voidTy, {i8PtrTy, i8PtrTy},
                                 false);
}

llvm::Function *LLVMIRGenerator::getOrDeclareYield() {
  return declareExternalFunction("yield", llvm::Type::getVoidTy(context), {},
                                 false);
}

void LLVMIRGenerator::generateYieldCall() {
  llvm::Function *yieldFn = module->getFunction("yield");
  builder.CreateCall(yieldFn, {});
}

llvm::Function *LLVMIRGenerator::getOrDeclareWaitGroupNew() {
  return declareExternalFunction("waitgroup_new",
                                 llvm::Type::getInt8PtrTy(context), {}, false);
}

llvm::Function *LLVMIRGenerator::getOrDeclareWaitGroupAdd() {
  return declareExternalFunction(
      "waitgroup_add", llvm::Type::getVoidTy(context),
      {llvm::Type::getInt8PtrTy(context), llvm::Type::getInt32Ty(context)},
      false);
}

llvm::Function *LLVMIRGenerator::getOrDeclareWaitGroupDone() {
  return declareExternalFunction("waitgroup_done",
                                 llvm::Type::getVoidTy(context),
                                 {llvm::Type::getInt8PtrTy(context)}, false);
}

llvm::Function *LLVMIRGenerator::getOrDeclareWaitGroupWait() {
  return declareExternalFunction("waitgroup_wait",
                                 llvm::Type::getVoidTy(context),
                                 {llvm::Type::getInt8PtrTy(context)}, false);
}

void LLVMIRGenerator::generateAllExternalFUnctions() {
  getOrDeclarePrintf();
  getOrDeclareScanf();
  getOrDeclareMalloc();
  getOrDeclareSpawn();
  getOrDeclareYield();
  getOrDeclareWaitGroupNew();
  getOrDeclareWaitGroupAdd();
  getOrDeclareWaitGroupDone();
  getOrDeclareWaitGroupWait();
}
} // namespace custom_ir
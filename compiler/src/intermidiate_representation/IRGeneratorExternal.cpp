#include <CommonExternalFunctions.hpp>
#include <IRGenerator.hpp>

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

llvm::Function *IRGenerator::getOrDeclareScanf() {

  llvm::Type *i8PtrTy = llvm::Type::getInt8PtrTy(context);

  return declareExternalFunction("scanf", llvm::Type::getInt32Ty(context),
                                 {i8PtrTy}, true);
}

llvm::Function *IRGenerator::getOrDeclareMalloc() {
  llvm::Type *i8PtrTy = llvm::Type::getInt8PtrTy(context);
  llvm::Type *i64Ty = llvm::Type::getInt64Ty(context);

  return declareExternalFunction("malloc", i8PtrTy, {i64Ty}, false);
}

llvm::Function *IRGenerator::getOrDeclareSpawn() {

  llvm::Type *voidTy = llvm::Type::getVoidTy(context);
  llvm::Type *i8PtrTy = llvm::Type::getInt8PtrTy(context);

  return declareExternalFunction("runtime_spawn", voidTy, {i8PtrTy, i8PtrTy},
                                 false);
}

llvm::Function *IRGenerator::getOrDeclareYield() {
  return declareExternalFunction("yield", llvm::Type::getVoidTy(context), {},
                                 false);
}

void IRGenerator::generateYieldCall() {
  llvm::Function *yieldFn = module->getFunction("yield");
  builder.CreateCall(yieldFn, {});
}

void IRGenerator::generateAllExternalFUnctions() {
  getOrDeclarePrintf();
  getOrDeclareScanf();
  getOrDeclareMalloc();
  getOrDeclareSpawn();
  getOrDeclareYield();
}

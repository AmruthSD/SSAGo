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

void IRGenerator::generateAllExternalFUnctions() {
  getOrDeclarePrintf();
  getOrDeclareScanf();
}

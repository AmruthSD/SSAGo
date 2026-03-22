#include <IRGenerator.hpp>

llvm::Value *IRGenerator::generateGoFunc(GoStmt *stmt) {
  std::string funcName = stmt->callee;
  if (external_functions.find(funcName) == external_functions.end())
    funcName = "__user_" + funcName;

  llvm::Function *func = module->getFunction(funcName);
  if (!func)
    return nullptr;
  llvm::Function *wrapper = getOrCreateWrapper(func);
  std::vector<llvm::Value *> argValues;
  std::vector<llvm::Type *> argTypes;

  for (auto &expr : stmt->arguments) {
    llvm::Value *val = expr->codegen(*this);
    argValues.push_back(val);
    argTypes.push_back(val->getType());
  }

  llvm::StructType *structType = llvm::StructType::get(context, argTypes);
  llvm::Constant *size = llvm::ConstantExpr::getSizeOf(structType);

  llvm::Function *mallocFunc = module->getFunction("malloc");
  llvm::Value *rawPtr = builder.CreateCall(mallocFunc, {size});

  llvm::Value *structPtr =
      builder.CreateBitCast(rawPtr, structType->getPointerTo());
  for (size_t i = 0; i < argValues.size(); i++) {
    llvm::Value *gep = builder.CreateStructGEP(structType, structPtr, i);
    builder.CreateStore(argValues[i], gep);
  }
  llvm::Value *voidPtr =
      builder.CreateBitCast(structPtr, builder.getInt8PtrTy());

  llvm::Function *spawnFunc = module->getFunction("runtime_spawn");
  llvm::Value *fnPtr = builder.CreateBitCast(wrapper, builder.getInt8PtrTy());
  return builder.CreateCall(spawnFunc, {fnPtr, voidPtr});
}

llvm::Function *IRGenerator::getOrCreateWrapper(llvm::Function *func) {
  if (wrapperMap.find(func) != wrapperMap.end())
    return wrapperMap[func];

  llvm::FunctionType *wrapperType = llvm::FunctionType::get(
      builder.getVoidTy(), {builder.getInt8PtrTy()}, false);

  std::string wrapperName = func->getName().str() + "_wrapper";

  llvm::Function *wrapper = llvm::Function::Create(
      wrapperType, llvm::Function::InternalLinkage, wrapperName, module.get());

  llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", wrapper);

  llvm::IRBuilder<> wrapperBuilder(entry);
  llvm::Value *argPtr = wrapper->getArg(0);

  std::vector<llvm::Type *> paramTypes;
  for (auto &arg : func->args()) {
    paramTypes.push_back(arg.getType());
  }

  llvm::StructType *structType = llvm::StructType::get(context, paramTypes);
  llvm::Value *typedPtr =
      wrapperBuilder.CreateBitCast(argPtr, structType->getPointerTo());
  std::vector<llvm::Value *> callArgs;

  for (size_t i = 0; i < paramTypes.size(); i++) {
    llvm::Value *gep = wrapperBuilder.CreateStructGEP(structType, typedPtr, i);

    llvm::Value *val = wrapperBuilder.CreateLoad(paramTypes[i], gep);

    callArgs.push_back(val);
  }

  wrapperBuilder.CreateCall(func, callArgs);
  wrapperBuilder.CreateRetVoid();

  wrapperMap[func] = wrapper;
  return wrapper;
}
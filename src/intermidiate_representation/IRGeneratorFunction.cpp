#include <CommonExternalFunctions.hpp>
#include <IRGenerator.hpp>
#include <llvm/IR/Verifier.h>

llvm::Value *IRGenerator::generateFunction(FunctionStmt *func) {

  if (namedValues.size() != 1)
    throw std::runtime_error("Unable to have function as non global");

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

llvm::Value *IRGenerator::generateReturn(ReturnStmt *stmt) {
  llvm::Function *function = builder.GetInsertBlock()->getParent();
  llvm::Type *returnType = function->getReturnType();

  if (returnType->isVoidTy()) {
    if (stmt->expr != nullptr) {
      llvm::errs() << "Cannot return a value from void function\n";
      return nullptr;
    }
    return builder.CreateRetVoid();
  }

  if (stmt->expr == nullptr) {
    llvm::errs() << "Non-void function must return a value\n";
    return nullptr;
  }

  llvm::Value *retValue = stmt->expr->codegen(*this);
  if (!retValue)
    return nullptr;

  if (retValue->getType() != returnType) {
    if (returnType->isDoubleTy() && retValue->getType()->isIntegerTy()) {
      retValue = builder.CreateSIToFP(retValue, returnType);
    } else if (returnType->isIntegerTy() && retValue->getType()->isDoubleTy()) {
      retValue = builder.CreateFPToSI(retValue, returnType);
    } else {
      llvm::errs() << "Return type mismatch\n";
      return nullptr;
    }
  }

  return builder.CreateRet(retValue);
}

llvm::Value *IRGenerator::generateFunctionCall(CallExpr *expr) {

  auto *var = dynamic_cast<VariableExpr *>(expr->callee.get());
  if (!var)
    throw std::runtime_error("Invalid function call target in codegen");

  std::string functionName = var->name;

  llvm::Function *function = module->getFunction(functionName);
  if (!function)
    throw std::runtime_error("LLVM function not found: " + functionName);

  std::vector<llvm::Value *> args;
  args.reserve(expr->arguments.size());

  for (auto &arg : expr->arguments) {
    llvm::Value *argVal = arg->codegen(*this);
    if (!argVal)
      throw std::runtime_error("Failed to generate argument in call to " +
                               functionName);
    args.push_back(argVal);
  }

  return builder.CreateCall(function, args, functionName + "_call");
}
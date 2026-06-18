#include <CommonExternalFunctions.hpp>
#include <LLVMIRGenerator.hpp>
#include <llvm/IR/Verifier.h>

namespace custom_ir {
llvm::Value *LLVMIRGenerator::generateFunction(Function *func) {

  if (namedValues.size() != 1)
    throw std::runtime_error("Unable to have function as non global");

  std::vector<llvm::Type *> paramTypes;
  for (auto &val : func->args)
    paramTypes.push_back(getLLVMType(val->dataType->pointee, context));

  llvm::FunctionType *functionType = llvm::FunctionType::get(
      getLLVMType(func->dataType, context), paramTypes, false);

  llvm::Function *function = llvm::Function::Create(
      functionType, llvm::Function::ExternalLinkage, func->value, module.get());

  currentFunction = function;
  unsigned idx = 0;
  for (auto &arg : function->args())
    arg.setName(func->args[idx++]->value);

  namedValues.emplace_back();
  idx = 0;

  std::cout << "function is created making arguments " << func->args.size()
            << std::endl;
  builder.SetInsertPoint(getOrCreateBasicBlock(func->blocks[0]));
  for (auto &arg : function->args()) {

    if (func->args[idx]->dataType->pointee == nullptr)
      std::cout << "argument type is nullptr " + func->args[idx]->value
                << std::endl;

    llvm::AllocaInst *alloca = builder.CreateAlloca(
        getLLVMType(func->args[idx]->dataType->pointee, context), nullptr,
        arg.getName());

    VariableValue *var = static_cast<VariableValue *>(func->args[idx]);
    if (var == nullptr)
      std::cout << "argument is nullptr" << std::endl;
    variableValuesLLVM[var->variable_id] = alloca;
    builder.CreateStore(&arg, alloca);
    namedValues.back()[arg.getName().str()] = alloca;

    idx++;
  }

  std::cout << "function is made generating instructions" << std::endl;

  for (auto &block : func->blocks) {
    block->llvm_codegen(this);
  }

  namedValues.pop_back();

  builder.ClearInsertionPoint();
  return function;
}

llvm::BasicBlock *LLVMIRGenerator::getOrCreateBasicBlock(BasicBlockIR *block) {
  llvm::BasicBlock *newBlock = basicBlocks[block];
  if (newBlock == nullptr)
    newBlock = llvm::BasicBlock::Create(context, block->name, currentFunction);

  return newBlock;
}

llvm::Value *LLVMIRGenerator::generateBlock(BasicBlockIR *block) {
  if (namedValues.size() == 1)
    throw std::runtime_error("Unable to have block as global");
  namedValues.emplace_back();

  llvm::BasicBlock *newBlock = getOrCreateBasicBlock(block);
  builder.SetInsertPoint(newBlock);
  for (auto &stmt : block->instructions)
    stmt->llvm_codegen(this);

  namedValues.pop_back();
  return nullptr;
}

llvm::Value *LLVMIRGenerator::generateFunctionCall(Instruction *inst) {

  // generateYieldCall();

  Function *funcValue = static_cast<Function *>(inst->operands.back());
  std::string functionName = funcValue->name;
  llvm::Function *function = module->getFunction(functionName);
  if (!function)
    throw std::runtime_error("LLVM function not found: " + functionName);

  std::vector<llvm::Value *> args;
  args.reserve(funcValue->args.size());
  int idx = 0;
  for (auto &arg : funcValue->args) {
    llvm::Value *argVal = inst->operands[idx]->llvm_codegen(this);
    idx++;
    if (!argVal)
      throw std::runtime_error("Failed to generate argument in call to " +
                               functionName);
    args.push_back(argVal);
  }

  llvm::FunctionType *ft = function->getFunctionType();
  std::string callName =
      ft->getReturnType()->isVoidTy() ? "" : functionName + "_call";

  return builder.CreateCall(function, args, callName);
}

llvm::Value *LLVMIRGenerator::generateExternalCall(Instruction *inst) {
  std::string functionName = inst->value;
  llvm::Function *function = module->getFunction(functionName);
  if (!function)
    throw std::runtime_error("LLVM function not found: " + functionName);

  std::vector<llvm::Value *> args;
  args.reserve(inst->operands.size() - 1);
  int idx = 0;
  for (idx = 0; idx < inst->operands.size() - 1; idx++) {
    llvm::Value *argVal = inst->operands[idx]->llvm_codegen(this);
    idx++;
    if (!argVal)
      throw std::runtime_error("Failed to generate argument in call to " +
                               functionName);
    args.push_back(argVal);
  }

  llvm::FunctionType *ft = function->getFunctionType();
  std::string callName =
      ft->getReturnType()->isVoidTy() ? "" : functionName + "_call";

  return builder.CreateCall(function, args, callName);
}

llvm::Value *
LLVMIRGenerator::generateSizeofExpr(ConstantSizeof *constantSizeOf) {
  llvm::Type *type = getLLVMType(constantSizeOf->dataType, context);
  uint64_t size = module->getDataLayout().getTypeAllocSize(type);

  return llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), size);
}
} // namespace custom_ir
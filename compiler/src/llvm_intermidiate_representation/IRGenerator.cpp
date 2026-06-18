#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include <CustomSSAGenerator.hpp>
#include <LLVMIRGenerator.hpp>
#include <iostream>
#include <llvm/IR/Verifier.h>

namespace custom_ir {

llvm::Type *LLVMIRGenerator::getLLVMType(Type *type,
                                         llvm::LLVMContext &context) {
  switch (type->base) {
  case DATA_TYPE::DATATYPE_INT:
    return llvm::Type::getInt32Ty(context);

  case DATA_TYPE::DATATYPE_FLOAT:
    return llvm::Type::getDoubleTy(context);

  case DATA_TYPE::DATATYPE_WAITGROUP:
    return llvm::Type::getInt8PtrTy(context);

  case DATA_TYPE::DATATYPE_VOID:
    return llvm::Type::getVoidTy(context);

  case DATA_TYPE::DATATYPE_POINTER: {
    llvm::Type *elemType = getLLVMType(type->pointee, context);
    return llvm::PointerType::getUnqual(elemType);
  }

  default:
    throw std::runtime_error("Unknown DATA_TYPE");
  }
}

LLVMIRGenerator::LLVMIRGenerator(SSAGenerator &ssaGenerator)
    : ssaGenerator(ssaGenerator),
      module(std::make_unique<llvm::Module>("compiler_module", context)),
      builder(context) {
  generateAllExternalFUnctions();
  ssaGenerator.module->llvm_codegen(this);
  module->print(llvm::errs(), nullptr);

  if (llvm::verifyModule(*module, &llvm::errs())) {
    llvm::errs() << "Module verification failed!\n";
    exit(1);
  }
}

llvm::Value *LLVMIRGenerator::generateModule(ModuleIR *module) {
  namedValues.emplace_back();
  generateGlobalVariables(module->globalDeclarations);
  for (auto &[_, func] : module->functions) {
    std::cout << "function declarations for " + func->name << std::endl;
    func->llvm_codegen(this);
  }
}

llvm::Value *LLVMIRGenerator::generateGlobalVariables(BasicBlockIR *block) {
  for (auto inst : block->instructions) {
    if (inst->opcode == Opcode::Global_Dec) {
      VariableValue *varValue = static_cast<VariableValue *>(inst->operands[1]);
      Constant *initValue = static_cast<Constant *>(inst->operands[0]);
      llvm::Type *type = getLLVMType(initValue->dataType, context);
      llvm::Constant *initConstValue = nullptr;
      initConstValue =
          llvm::dyn_cast<llvm::Constant>(generateLiteral(initValue));
      if (!initConstValue) {
        llvm::errs() << "Global initializer must be constant!\n";
        exit(1);
      }

      auto *global = new llvm::GlobalVariable(
          *module, type, false, llvm::GlobalValue::ExternalLinkage,
          initConstValue, varValue->value);

      variableValuesLLVM[varValue->variable_id] = global;
      namedValues.back()[varValue->value] = global;
      return global;
    }
  }
}

llvm::Value *LLVMIRGenerator::getVariablePointer(std::string &name) {
  for (auto it = namedValues.rbegin(); it != namedValues.rend(); ++it) {
    auto found = it->find(name);
    if (found != it->end())
      return found->second;
  }

  return nullptr;
}

llvm::LLVMContext &LLVMIRGenerator::getContext() { return context; }

llvm::Module *LLVMIRGenerator::getModule() { return module.get(); }

llvm::IRBuilder<> &LLVMIRGenerator::getBuilder() { return builder; }

llvm::Value *LLVMIRGenerator::generateLiteral(Constant *expr) {
  DATA_TYPE dataType = expr->dataType->base;
  switch (dataType) {
  case DATA_TYPE::DATATYPE_INT:
    return llvm::ConstantInt::get(getLLVMType(expr->dataType, context),
                                  std::stoi(expr->value), true);

  case DATA_TYPE::DATATYPE_FLOAT:
    return llvm::ConstantFP::get(getLLVMType(expr->dataType, context),
                                 std::stof(expr->value));

  case DATA_TYPE::DATATYPE_STRING:
    return builder.CreateGlobalStringPtr(expr->value);

  case DATA_TYPE::DATATYPE_BOOL:
    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(context),
                                  expr->value == "1");
  }

  throw std::runtime_error("Unknown literal type");
}

llvm::Value *LLVMIRGenerator::getTempValue(TempValue *val) {
  llvm::Value *temp = tempValues[val->value];
  if (val == nullptr)
    throw std::runtime_error("temp value not yet created for " + val->value);
  return temp;
}

llvm::Value *LLVMIRGenerator::getVariableValue(VariableValue *var) {
  if (variableValuesLLVM[var->variable_id] == nullptr)
    throw std::runtime_error("no varibales with " + var->value);

  return variableValuesLLVM[var->variable_id];
}

llvm::Value *LLVMIRGenerator::generateCast(Instruction *inst) {

  llvm::Value *val = inst->operands[0]->llvm_codegen(this);
  Value *res = inst->operands[1];
  if (!val)
    return nullptr;

  llvm::Type *targetType = getLLVMType(res->dataType, context);
  Opcode op = inst->opcode;
  if (op == Opcode::Cast_FLOAT) {

    return tempValues[res->value] =
               builder.CreateSIToFP(val, targetType, res->value);
  }
  if (op == Opcode::Cast_INT) {

    return tempValues[res->value] =
               builder.CreateFPToSI(val, targetType, res->value);
  }

  throw std::runtime_error("Unsupported cast");
}

llvm::Value *LLVMIRGenerator::generateAlloca(Instruction *inst) {
  std::cout << "Alloca instruction generation" << std::endl;
  VariableValue *var = static_cast<VariableValue *>(inst->operands[0]);
  llvm::Type *varType = getLLVMType(var->dataType->pointee, context);
  if (variableValuesLLVM[var->variable_id] != nullptr)
    return variableValuesLLVM[var->variable_id];
  llvm::AllocaInst *alloca = builder.CreateAlloca(varType, nullptr, var->value);

  return variableValuesLLVM[var->variable_id] = alloca;
}

llvm::Value *LLVMIRGenerator::generateLoad(Instruction *inst) {
  std::cout << "Load instruction generation" << std::endl;
  Value *tempVal = inst->operands[1];
  llvm::Value *varVal = inst->operands[0]->llvm_codegen(this);
  llvm::Type *elementType = varVal->getType()->getPointerElementType();

  return tempValues[tempVal->value] =
             builder.CreateLoad(elementType, varVal, tempVal->value);
}

llvm::Value *LLVMIRGenerator::generateStore(Instruction *inst) {
  std::cout << "Store instruction generation" << std::endl;
  llvm::Value *temp = inst->operands[0]->llvm_codegen(this);
  llvm::Value *varValue = inst->operands[1]->llvm_codegen(this);
  return builder.CreateStore(temp, varValue);
}

void LLVMIRGenerator::optimizeModule() {
  llvm::PassBuilder passBuilder;

  llvm::LoopAnalysisManager LAM;
  llvm::FunctionAnalysisManager FAM;
  llvm::CGSCCAnalysisManager CGAM;
  llvm::ModuleAnalysisManager MAM;

  passBuilder.registerModuleAnalyses(MAM);
  passBuilder.registerCGSCCAnalyses(CGAM);
  passBuilder.registerFunctionAnalyses(FAM);
  passBuilder.registerLoopAnalyses(LAM);
  passBuilder.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  llvm::ModulePassManager MPM =
      passBuilder.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);

  MPM.run(*module, MAM);
}

} // namespace custom_ir
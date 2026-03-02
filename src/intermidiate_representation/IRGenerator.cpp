#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include <IRGenerator.hpp>
#include <SemanticAnalyser.hpp>
#include <iostream>
#include <llvm/IR/Verifier.h>

llvm::Type *IRGenerator::getLLVMType(DATA_TYPE type,
                                     llvm::LLVMContext &context) {
  switch (type) {
  case DATA_TYPE::DATATYPE_INT:
    return llvm::Type::getInt32Ty(context);

  case DATA_TYPE::DATATYPE_FLOAT:
    return llvm::Type::getDoubleTy(context);

  case DATA_TYPE::DATATYPE_VOID:
    return llvm::Type::getVoidTy(context);

  default:
    throw std::runtime_error("Unknown DATA_TYPE");
  }
}

IRGenerator::IRGenerator(SemanticAnalyser &semanticAnalyser)
    : semanticAnalyser(semanticAnalyser),
      module(std::make_unique<llvm::Module>("compiler_module", context)),
      builder(context) {
  generateAllExternalFUnctions();
  namedValues.emplace_back();
  semanticAnalyser.ast.get()->codegen(*this);

  // module->print(llvm::errs(), nullptr);

  if (llvm::verifyModule(*module, &llvm::errs())) {
    llvm::errs() << "Module verification failed!\n";
    exit(1);
  }
}

llvm::Value *IRGenerator::getVariablePointer(std::string &name) {
  for (auto it = namedValues.rbegin(); it != namedValues.rend(); ++it) {
    auto found = it->find(name);
    if (found != it->end())
      return found->second;
  }

  return nullptr;
}

llvm::LLVMContext &IRGenerator::getContext() { return context; }

llvm::Module *IRGenerator::getModule() { return module.get(); }

llvm::IRBuilder<> &IRGenerator::getBuilder() { return builder; }

llvm::Value *IRGenerator::generateProgram(Program *program) {
  for (auto &stmt : program->statements) {
    stmt->codegen(*this);
  }

  return nullptr;
}

llvm::Value *IRGenerator::generateExpressionStmt(ExpressionStmt *stmt) {
  if (namedValues.size() == 1)
    throw std::runtime_error("Unable to have expression as global");

  return stmt->expression->codegen(*this);
}

llvm::Value *IRGenerator::generateLiteral(LiteralExpr *expr) {
  DATA_TYPE dataType = expr->dataType;
  switch (dataType) {
  case DATA_TYPE::DATATYPE_INT:
    return llvm::ConstantInt::get(getLLVMType(dataType, context),
                                  std::stoi(expr->value), true);

  case DATA_TYPE::DATATYPE_FLOAT:
    return llvm::ConstantFP::get(getLLVMType(dataType, context),
                                 std::stof(expr->value));

  case DATA_TYPE::DATATYPE_STRING:
    return builder.CreateGlobalStringPtr(expr->value);
  }

  throw std::runtime_error("Unknown literal type");
}

llvm::Value *IRGenerator::generateCast(CastExpr *expr) {

  llvm::Value *val = expr->expr->codegen(*this);
  if (!val)
    return nullptr;

  llvm::Type *targetType = getLLVMType(expr->dataType, context);

  if (val->getType() == targetType)
    return val;
  if (val->getType()->isIntegerTy() && targetType->isDoubleTy()) {

    return builder.CreateSIToFP(val, targetType, "intToDouble");
  }
  if (val->getType()->isDoubleTy() && targetType->isIntegerTy()) {

    return builder.CreateFPToSI(val, targetType, "doubleToInt");
  }

  throw std::runtime_error("Unsupported cast");
}

llvm::Value *IRGenerator::generateBinary(BinaryExpr *expr) {

  if (expr->op == TOKEN_TYPE::ASSIGN) {

    auto *var = dynamic_cast<VariableExpr *>(expr->left.get());
    if (!var)
      throw std::runtime_error("Left side of assignment must be a variable");

    llvm::Value *ptr = getVariablePointer(var->name);
    if (!ptr)
      throw std::runtime_error("Unknown variable: " + var->name);

    llvm::Value *value = expr->right->codegen(*this);
    if (!value)
      return nullptr;

    builder.CreateStore(value, ptr);
    return value;
  }

  llvm::Value *L = expr->left->codegen(*this);
  llvm::Value *R = expr->right->codegen(*this);

  if (!L || !R)
    return nullptr;

  if (L->getType()->isDoubleTy()) {

    switch (expr->op) {

    case TOKEN_TYPE::PLUS:
      return builder.CreateFAdd(L, R, "faddtmp");

    case TOKEN_TYPE::MINUS:
      return builder.CreateFSub(L, R, "fsubtmp");

    case TOKEN_TYPE::ASTERISK:
      return builder.CreateFMul(L, R, "fmultmp");

    case TOKEN_TYPE::SLASH:
      return builder.CreateFDiv(L, R, "fdivtmp");

    // ===== Comparisons (double) =====
    case TOKEN_TYPE::EQUAL:
      return builder.CreateFCmpOEQ(L, R, "cmptmp");

    case TOKEN_TYPE::NOT_EQUAL:
      return builder.CreateFCmpONE(L, R, "cmptmp");

    case TOKEN_TYPE::LESS:
      return builder.CreateFCmpOLT(L, R, "cmptmp");

    case TOKEN_TYPE::GREATER:
      return builder.CreateFCmpOGT(L, R, "cmptmp");

    default:
      return nullptr;
    }
  }

  switch (expr->op) {

  case TOKEN_TYPE::PLUS:
    return builder.CreateAdd(L, R, "addtmp");

  case TOKEN_TYPE::MINUS:
    return builder.CreateSub(L, R, "subtmp");

  case TOKEN_TYPE::ASTERISK:
    return builder.CreateMul(L, R, "multmp");

  case TOKEN_TYPE::SLASH:
    return builder.CreateSDiv(L, R, "divtmp");

  // ===== Comparisons (int) =====
  case TOKEN_TYPE::EQUAL:
    return builder.CreateICmpEQ(L, R, "cmptmp");

  case TOKEN_TYPE::NOT_EQUAL:
    return builder.CreateICmpNE(L, R, "cmptmp");

  case TOKEN_TYPE::LESS:
    return builder.CreateICmpSLT(L, R, "cmptmp");

  case TOKEN_TYPE::GREATER:
    return builder.CreateICmpSGT(L, R, "cmptmp");

  // ===== Logical ops =====
  case TOKEN_TYPE::AND: {
    L = builder.CreateICmpNE(L, llvm::ConstantInt::get(L->getType(), 0));
    R = builder.CreateICmpNE(R, llvm::ConstantInt::get(R->getType(), 0));
    return builder.CreateAnd(L, R, "andtmp");
  }

  case TOKEN_TYPE::OR: {
    L = builder.CreateICmpNE(L, llvm::ConstantInt::get(L->getType(), 0));
    R = builder.CreateICmpNE(R, llvm::ConstantInt::get(R->getType(), 0));
    return builder.CreateOr(L, R, "ortmp");
  }

  default:
    return nullptr;
  }
}

llvm::Value *IRGenerator::generateVariable(VariableExpr *expr) {
  llvm::Value *ptr = nullptr;

  for (auto it = namedValues.rbegin(); it != namedValues.rend(); ++it) {
    auto found = it->find(expr->name);
    if (found != it->end()) {
      ptr = found->second;
      break;
    }
  }

  if (!ptr) {
    llvm::errs() << "Unknown variable: " << expr->name << "\n";
    return nullptr;
  }

  if (ptr->getType()->isPointerTy()) {
    llvm::Type *elementType = ptr->getType()->getPointerElementType();

    return builder.CreateLoad(elementType, ptr, expr->name + "_val");
  }

  return ptr;
}

llvm::Value *IRGenerator::generateDeclaration(DeclarationStmt *stmt) {
  llvm::Type *type = getLLVMType(stmt->dataType, context);

  if (!builder.GetInsertBlock()) {

    llvm::Constant *initValue = nullptr;

    if (stmt->expr) {
      llvm::Value *val = stmt->expr->codegen(*this);

      initValue = llvm::dyn_cast<llvm::Constant>(val);
      if (!initValue) {
        llvm::errs() << "Global initializer must be constant!\n";
        exit(1);
      }

    } else {
      initValue = llvm::Constant::getNullValue(type);
    }

    auto *global = new llvm::GlobalVariable(*module, type, false,
                                            llvm::GlobalValue::ExternalLinkage,
                                            initValue, stmt->identifier);

    namedValues.back()[stmt->identifier] = global;
    return global;
  } else {
    llvm::Value *initValue = nullptr;

    llvm::AllocaInst *alloca = builder.CreateAlloca(
        getLLVMType(stmt->dataType, context), nullptr, stmt->identifier);

    if (stmt->expr != nullptr) {
      initValue = stmt->expr->codegen(*this);
    } else {
      initValue =
          llvm::ConstantInt::get(getLLVMType(stmt->dataType, context), 0);
    }

    builder.CreateStore(initValue, alloca);

    namedValues.back()[stmt->identifier] = alloca;

    return alloca;
  }
}

void IRGenerator::optimizeModule() {
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
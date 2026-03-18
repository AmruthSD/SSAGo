#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include <IRGenerator.hpp>
#include <SemanticAnalyser.hpp>
#include <iostream>
#include <llvm/IR/Verifier.h>

llvm::Type *IRGenerator::getLLVMType(Type *type, llvm::LLVMContext &context) {
  switch (type->base) {
  case DATA_TYPE::DATATYPE_INT:
    return llvm::Type::getInt32Ty(context);

  case DATA_TYPE::DATATYPE_FLOAT:
    return llvm::Type::getDoubleTy(context);

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

    if (!expr->left->isLValue())
      throw std::runtime_error("Left side of assignment is not assignable");

    llvm::Value *ptr = expr->left->codegenLValue(*this);
    llvm::Value *value = expr->right->codegen(*this);
    if (!value)
      return nullptr;

    llvm::Type *ptrElemTy = ptr->getType()->getPointerElementType();
    if (value->getType() != ptrElemTy) {

      if (value->getType()->isPointerTy() && ptrElemTy->isPointerTy()) {
        value = builder.CreateBitCast(value, ptrElemTy);
      } else if (value->getType()->isIntegerTy() && ptrElemTy->isIntegerTy()) {
        unsigned fromBits = value->getType()->getIntegerBitWidth();
        unsigned toBits = ptrElemTy->getIntegerBitWidth();

        if (fromBits < toBits) {
          value = builder.CreateZExt(value, ptrElemTy);
        } else if (fromBits > toBits) {
          value = builder.CreateTrunc(value, ptrElemTy);
        }
      }

      else {
        llvm::errs() << "Invalid type mismatch in assignment\n";
        return nullptr;
      }
    }

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

llvm::Value *IRGenerator::generateUnaryExpr(UnaryExpr *expr) {
  switch (expr->op) {

  case TOKEN_TYPE::ASTERISK: {
    llvm::Value *ptr = generateUnaryExprLValue(expr);
    if (!ptr)
      return nullptr;

    llvm::Type *elementType = ptr->getType()->getPointerElementType();

    return builder.CreateLoad(elementType, ptr, "deref_val");
  }
  case TOKEN_TYPE::AMPERSAND: {
    return expr->operand->codegenLValue(*this);
  }

  default:
    llvm::errs() << "Unsupported unary operator\n";
    return nullptr;
  }
}

llvm::Value *IRGenerator::generateUnaryExprLValue(UnaryExpr *expr) {
  switch (expr->op) {

  case TOKEN_TYPE::ASTERISK: {
    llvm::Value *ptr = expr->operand->codegen(*this);
    if (!ptr)
      return nullptr;

    if (!ptr->getType()->isPointerTy()) {
      llvm::errs() << "Cannot dereference non-pointer\n";
      return nullptr;
    }

    return ptr;
  }

  default:
    llvm::errs() << "Unsupported unary operator for lvalue\n";
    return nullptr;
  }
}

llvm::Value *IRGenerator::generateVariable(VariableExpr *expr) {
  llvm::Value *ptr = generateVariableLValue(expr);
  if (!ptr)
    return nullptr;

  llvm::Type *elementType = ptr->getType()->getPointerElementType();
  return builder.CreateLoad(elementType, ptr, expr->name + "_val");
}

llvm::Value *IRGenerator::generateVariableLValue(VariableExpr *expr) {
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
    llvm::Type *varType = getLLVMType(stmt->dataType, context);

    llvm::AllocaInst *alloca =
        builder.CreateAlloca(varType, nullptr, stmt->identifier);

    llvm::Value *initValue = nullptr;

    if (stmt->expr != nullptr) {
      initValue = stmt->expr->codegen(*this);
      if (initValue->getType() != varType) {
        if (initValue->getType()->isPointerTy() && varType->isPointerTy()) {
          initValue = builder.CreateBitCast(initValue, varType);
        } else if (initValue->getType()->isIntegerTy() &&
                   varType->isIntegerTy()) {
          unsigned fromBits = initValue->getType()->getIntegerBitWidth();
          unsigned toBits = varType->getIntegerBitWidth();
          if (fromBits < toBits) {
            initValue = builder.CreateZExt(initValue, varType);
          } else if (fromBits > toBits) {
            initValue = builder.CreateTrunc(initValue, varType);
          }
        } else {
          llvm::errs() << "Type mismatch in variable initialization\n";
          return nullptr;
        }
      }
    } else {
      if (varType->isPointerTy()) {
        initValue = llvm::ConstantPointerNull::get(
            llvm::cast<llvm::PointerType>(varType));
      } else if (varType->isIntegerTy()) {
        initValue = llvm::ConstantInt::get(varType, 0);
      } else if (varType->isDoubleTy()) {
        initValue = llvm::ConstantFP::get(varType, 0.0);
      } else {
        llvm::errs() << "Unsupported type for default initialization\n";
        return nullptr;
      }
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
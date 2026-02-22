#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include <IRGenerator.hpp>
#include <SemanticAnalyser.hpp>
#include <llvm/IR/Verifier.h>

llvm::Type *getLLVMType(DATA_TYPE type, llvm::LLVMContext &context) {
  switch (type) {
  case DATA_TYPE::DATATYPE_INT:
    return llvm::Type::getInt32Ty(context);

  case DATA_TYPE::DATATYPE_FLOAT:
    return llvm::Type::getDoubleTy(context);

  default:
    throw std::runtime_error("Unknown DATA_TYPE");
  }
}

IRGenerator::IRGenerator(SemanticAnalyser &semanticAnalyser)
    : semanticAnalyser(semanticAnalyser),
      module(std::make_unique<llvm::Module>("compiler_module", context)),
      builder(context) {
  semanticAnalyser.ast.get()->codegen(*this);
  if (llvm::verifyModule(*module, &llvm::errs())) {
    llvm::errs() << "Module verification failed!\n";
    exit(1);
  }
}

llvm::LLVMContext &IRGenerator::getContext() { return context; }

llvm::Module *IRGenerator::getModule() { return module.get(); }

llvm::IRBuilder<> &IRGenerator::getBuilder() { return builder; }

llvm::Value *IRGenerator::generateProgram(Program *program) {
  llvm::FunctionType *funcType =
      llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false);

  llvm::Function *mainFunc = llvm::Function::Create(
      funcType, llvm::Function::ExternalLinkage, "main", module.get());

  llvm::BasicBlock *entry =
      llvm::BasicBlock::Create(context, "entry", mainFunc);

  builder.SetInsertPoint(entry);

  for (auto &stmt : program->statements) {
    stmt->codegen(*this);
  }

  builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0));

  return mainFunc;
}

llvm::Value *IRGenerator::generateExpressionStmt(ExpressionStmt *stmt) {
  return stmt->expression->codegen(*this);
}

llvm::Value *IRGenerator::generateLiteral(LiteralExpr *expr) {
  int value = std::stoi(expr->value);
  DATA_TYPE dataType = expr->dataType;
  switch (dataType) {
  case DATA_TYPE::DATATYPE_INT:
    return llvm::ConstantInt::get(getLLVMType(dataType, context),
                                  std::stoi(expr->value), true);

  case DATA_TYPE::DATATYPE_FLOAT:
    return llvm::ConstantFP::get(getLLVMType(dataType, context),
                                 std::stof(expr->value));
  }

  throw std::runtime_error("Unknown literal type");
}

llvm::Value *IRGenerator::generateBinary(BinaryExpr *expr) {
  llvm::Value *L = expr->left->codegen(*this);
  llvm::Value *R = expr->right->codegen(*this);

  if (!L || !R)
    return nullptr;

  llvm::Type *doubleTy = llvm::Type::getDoubleTy(context);

  bool LIsDouble = L->getType()->isDoubleTy();
  bool RIsDouble = R->getType()->isDoubleTy();

  if (LIsDouble || RIsDouble) {

    if (!LIsDouble)
      L = builder.CreateSIToFP(L, doubleTy, "intToDoubleL");

    if (!RIsDouble)
      R = builder.CreateSIToFP(R, doubleTy, "intToDoubleR");

    switch (expr->op) {
    case TOKEN_TYPE::PLUS:
      return builder.CreateFAdd(L, R, "faddtmp");

    case TOKEN_TYPE::MINUS:
      return builder.CreateFSub(L, R, "fsubtmp");

    case TOKEN_TYPE::ASTERISK:
      return builder.CreateFMul(L, R, "fmultmp");

    case TOKEN_TYPE::SLASH:
      return builder.CreateFDiv(L, R, "fdivtmp");

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

  default:
    return nullptr;
  }
}

llvm::Value *IRGenerator::generateVariable(VariableExpr *expr) {
  llvm::Value *alloca = namedValues[expr->name];

  if (!alloca)
    return nullptr;

  return builder.CreateLoad(getLLVMType(expr->dataType, context), alloca,
                            expr->name + "_val");
}

llvm::Value *IRGenerator::generateDeclaration(DeclarationStmt *stmt) {
  llvm::Value *initValue = nullptr;

  llvm::AllocaInst *alloca = builder.CreateAlloca(
      getLLVMType(stmt->dataType, context), nullptr, stmt->identifier);

  if (stmt->expr != nullptr) {
    initValue = stmt->expr->codegen(*this);
  } else {
    initValue =
        llvm::ConstantInt::get(getLLVMType(stmt->expr->dataType, context), 0);
  }

  builder.CreateStore(initValue, alloca);

  namedValues[stmt->identifier] = alloca;

  return alloca;
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
#include <IRGenerator.hpp>
#include <SemanticAnalyser.hpp>
#include <llvm/IR/Verifier.h>

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

llvm::Value *IRGenerator::generateNumber(NumberExpr *expr) {
  int value = std::stoi(expr->value);

  return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), value, true);
}

llvm::Value *IRGenerator::generateBinary(BinaryExpr *expr) {
  llvm::Value *L = expr->left->codegen(*this);
  llvm::Value *R = expr->right->codegen(*this);

  if (!L || !R)
    return nullptr;

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

  return builder.CreateLoad(llvm::Type::getInt32Ty(context), alloca,
                            expr->name + "_val");
}

llvm::Value *IRGenerator::generateDeclaration(DeclarationStmt *stmt) {
  llvm::Value *initValue = nullptr;

  llvm::AllocaInst *alloca = builder.CreateAlloca(
      llvm::Type::getInt32Ty(context), nullptr, stmt->identifier);

  if (stmt->expr != nullptr) {
    initValue = stmt->expr->codegen(*this);
  } else {
    initValue = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
  }

  builder.CreateStore(initValue, alloca);

  namedValues[stmt->identifier] = alloca;

  return alloca;
}
#pragma once

#include <SemanticAnalyser.hpp>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

class IRGenerator {
private:
  SemanticAnalyser &semanticAnalyser;

  llvm::LLVMContext context;
  std::unique_ptr<llvm::Module> module;
  llvm::IRBuilder<> builder;
  std::unordered_map<std::string, llvm::Value *> namedValues;

public:
  IRGenerator(SemanticAnalyser &semanticAnalyser);

  llvm::LLVMContext &getContext();
  llvm::Module *getModule();
  llvm::IRBuilder<> &getBuilder();

  void optimizeModule();

  llvm::Value *generateProgram(Program *);
  llvm::Value *generateExpressionStmt(ExpressionStmt *);
  llvm::Value *generateBinary(BinaryExpr *);
  llvm::Value *generateLiteral(LiteralExpr *);
  llvm::Value *generateVariable(VariableExpr *);
  llvm::Value *generateDeclaration(DeclarationStmt *);
};

#pragma once

#include <AST.hpp>
#include <Parser.hpp>
#include <memory>
#include <stack>
#include <unordered_map>

struct SymbolTableEntry {
  DATA_TYPE dataType;
  bool function = false;
  int block_number = 0;
};

class SemanticAnalyser {
private:
  Parser parser;
  int block_number = 0;
  std::unordered_map<std::string, SymbolTableEntry> symbolTable;

public:
  std::unique_ptr<Program> ast;
  SemanticAnalyser(Parser &parser);
  void analyseAST();
  void analyseProgram(Program *program);
  void analyseDeclarationStmt(DeclarationStmt *stmt);
  void analyseFunctionStmt(FunctionStmt *stmt);
  void analyseBlock(BlockStmt *stmt);
  void analyseExpressionStmt(ExpressionStmt *stmt);
  DATA_TYPE analyseBinaryExpr(BinaryExpr *expr);
  DATA_TYPE analyseLiteralExpr(LiteralExpr *expr);
  DATA_TYPE analyseVariableExpr(VariableExpr *expr);
};

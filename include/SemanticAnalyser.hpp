#pragma once

#include <AST.hpp>
#include <Parser.hpp>
#include <memory>
#include <unordered_map>

struct SymbolTableEntry {
  DATA_TYPE dataType;
};

class SemanticAnalyser {
private:
  Parser parser;

  std::unique_ptr<Program> ast;
  std::unordered_map<std::string, SymbolTableEntry> symbolTable;

public:
  SemanticAnalyser(Parser &parser);
  void analyseAST();
  void analyseProgram(Program *program);
  void analyseDeclarationStmt(DeclarationStmt *stmt);
  void analyseExpressionStmt(ExpressionStmt *stmt);
  DATA_TYPE analyseBinaryExpr(BinaryExpr *expr);
  DATA_TYPE analyseNumberExpr(NumberExpr *expr);
  DATA_TYPE analyseVariableExpr(VariableExpr *expr);
};

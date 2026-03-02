#pragma once

#include <AST.hpp>
#include <Parser.hpp>
#include <memory>
#include <stack>
#include <unordered_map>

struct SymbolTableEntry {
  DATA_TYPE dataType;
  int block_number = 0;
};

struct FunctionSymbolTableEntry {
  DATA_TYPE returnType;
  std::vector<DATA_TYPE> argumentsTypes;
};

class SemanticAnalyser {
private:
  Parser parser;
  int block_number = 0;
  std::unordered_map<std::string, SymbolTableEntry> symbolTable;
  std::unordered_map<std::string, FunctionSymbolTableEntry> functionSymbolTable;
  DATA_TYPE current_function_type = DATA_TYPE::DATATYPE_INT;

public:
  std::unique_ptr<Program> ast;
  SemanticAnalyser(Parser &parser);
  void analyseAST();
  void analyseProgram(Program *program);
  void analyseDeclarationStmt(DeclarationStmt *stmt);
  void analyseFunctionStmt(FunctionStmt *stmt);
  void analyseBlock(BlockStmt *stmt);
  void analyseReturn(ReturnStmt *stmt);
  void analyseExpressionStmt(ExpressionStmt *stmt);
  DATA_TYPE analyseBinaryExpr(BinaryExpr *expr);
  DATA_TYPE analyseLiteralExpr(LiteralExpr *expr);
  DATA_TYPE analyseVariableExpr(VariableExpr *expr);
  DATA_TYPE analyseFunctionCall(CallExpr *expr);
};

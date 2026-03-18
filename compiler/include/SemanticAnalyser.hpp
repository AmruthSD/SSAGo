#pragma once

#include <AST.hpp>
#include <CommonExternalFunctions.hpp>
#include <Parser.hpp>
#include <memory>
#include <stack>
#include <unordered_map>

struct SymbolTableEntry {
  Type *dataType;
  int block_number = 0;
};

struct FunctionSymbolTableEntry {
  Type *returnType;
  std::vector<Type *> argumentsTypes;
};

class SemanticAnalyser {
private:
  Parser parser;
  int block_number = 0;
  int loop_number = 0;
  std::unordered_map<std::string, SymbolTableEntry> symbolTable;
  std::unordered_map<std::string, FunctionSymbolTableEntry> functionSymbolTable;
  Type *current_function_type = new Type{DATA_TYPE::DATATYPE_INT, nullptr};

public:
  std::unique_ptr<Program> ast;
  SemanticAnalyser(Parser &parser);
  void analyseAST();
  void analyseProgram(Program *program);
  void analyseDeclarationStmt(DeclarationStmt *stmt);
  void analyseFunctionStmt(FunctionStmt *stmt);
  void analyseBlock(BlockStmt *stmt);
  void analyseReturn(ReturnStmt *stmt);
  void analyseIfElse(IfStmt *stmt);
  void analyseExpressionStmt(ExpressionStmt *stmt);
  void analyseWhile(WhileStmt *stmt);
  void analyseBreak(BreakStmt *stmt);
  void analyseContinue(ContinueStmt *stmt);
  Type *analyseBinaryExpr(BinaryExpr *expr);
  Type *analyseLiteralExpr(LiteralExpr *expr);
  Type *analyseVariableExpr(VariableExpr *expr);
  Type *analyseUnaryExpr(UnaryExpr *expr);
  Type *analyseFunctionCall(CallExpr *expr);
  Type *analyseSizeofExpr(SizeofExpr *expr);
};

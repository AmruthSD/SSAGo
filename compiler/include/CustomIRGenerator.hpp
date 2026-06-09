#pragma once

#include <AST.hpp>
#include <CustomIR.hpp>
#include <SemanticAnalyser.hpp>

namespace custom_ir {

class IRGenerator {
  ModuleIR *module;
  SemanticAnalyser &semanticAnalyser;

public:
  IRGenerator(SemanticAnalyser &semanticAnalyser);

  Value *generateProgram(Program *);
  Value *getVariablePointer(std::string &name);
  Value *generateExpressionStmt(ExpressionStmt *);
  Value *generateFunction(FunctionStmt *);
  Value *generateBlock(BlockStmt *);
  Value *generateBinary(BinaryExpr *);
  Value *generateLiteral(LiteralExpr *);
  Value *generateCast(CastExpr *);
  Value *generateVariable(VariableExpr *);
  Value *generateVariableLValue(VariableExpr *);
  Value *generateDeclaration(DeclarationStmt *);
  Value *generateReturn(ReturnStmt *);
  Value *generateIfElse(IfStmt *);
  Value *generateWhile(WhileStmt *);
  Value *generateBreak(BreakStmt *);
  Value *generateContinue(ContinueStmt *);
  Value *generateGoFunc(GoStmt *);
  Value *generateFunctionCall(CallExpr *);
  Value *generateUnaryExpr(UnaryExpr *);
  Value *generateUnaryExprLValue(UnaryExpr *);
  Value *generateSizeofExpr(SizeofExpr *);
};
} // namespace custom_ir

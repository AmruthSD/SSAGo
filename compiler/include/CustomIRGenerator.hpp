#pragma once

#include <AST.hpp>
#include <CustomIR.hpp>
#include <CustomIRBuilder.hpp>
#include <SemanticAnalyser.hpp>
#include <unordered_map>
#include <vector>

namespace custom_ir {

class IRGenerator {
  ModuleIR *module;
  SemanticAnalyser &semanticAnalyser;
  IRBuilder builder;

  std::vector<std::unordered_map<std::string, custom_ir::Value *>> namedValues;

  const std::map<TOKEN_TYPE, Opcode> intOpcodeMap = {
      {TOKEN_TYPE::PLUS, Opcode::Add},
      {TOKEN_TYPE::MINUS, Opcode::Sub},
      {TOKEN_TYPE::ASTERISK, Opcode::Mul},
      {TOKEN_TYPE::SLASH, Opcode::Div},

      {TOKEN_TYPE::EQUAL, Opcode::ICmpEQ},
      {TOKEN_TYPE::NOT_EQUAL, Opcode::ICmpNE},
      {TOKEN_TYPE::LESS, Opcode::ICmpLT},
      {TOKEN_TYPE::GREATER, Opcode::ICmpGT},

      {TOKEN_TYPE::AND, Opcode::And},
      {TOKEN_TYPE::OR, Opcode::Or},
  };

  const std::map<TOKEN_TYPE, Opcode> floatOpcodeMap = {
      {TOKEN_TYPE::PLUS, Opcode::FAdd},
      {TOKEN_TYPE::MINUS, Opcode::FSub},
      {TOKEN_TYPE::ASTERISK, Opcode::FMul},
      {TOKEN_TYPE::SLASH, Opcode::FDiv},

      {TOKEN_TYPE::EQUAL, Opcode::FCmpEQ},
      {TOKEN_TYPE::NOT_EQUAL, Opcode::FCmpNE},
      {TOKEN_TYPE::LESS, Opcode::FCmpLT},
      {TOKEN_TYPE::GREATER, Opcode::FCmpGT},
  };

  const std::map<TOKEN_TYPE, std::string> tempNameMap = {
      {TOKEN_TYPE::PLUS, "addtmp"},     {TOKEN_TYPE::MINUS, "subtmp"},
      {TOKEN_TYPE::ASTERISK, "multmp"}, {TOKEN_TYPE::SLASH, "divtmp"},

      {TOKEN_TYPE::EQUAL, "eqtmp"},     {TOKEN_TYPE::NOT_EQUAL, "netmp"},
      {TOKEN_TYPE::LESS, "lttmp"},      {TOKEN_TYPE::GREATER, "gttmp"},

      {TOKEN_TYPE::AND, "andtmp"},      {TOKEN_TYPE::OR, "ortmp"}};

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

#include <AST.hpp>
#include <CustomIRGenerator.hpp>
#include <SemanticAnalyser.hpp>

BinaryExpr::BinaryExpr(TOKEN_TYPE oper, std::unique_ptr<Expr> lhs,
                       std::unique_ptr<Expr> rhs)
    : op(oper), left(std::move(lhs)), right(std::move(rhs)) {}

Type *BinaryExpr::analyse(SemanticAnalyser &analyser) {
  return dataType = analyser.analyseBinaryExpr(this);
}

custom_ir::Value *BinaryExpr::codegen(custom_ir::IRGenerator &irGen) {
  return irGen.generateBinary(this);
}

LiteralExpr::LiteralExpr(const std::string &val, Type *dataType) : value(val) {
  this->dataType = dataType;
}

Type *LiteralExpr::analyse(SemanticAnalyser &analyser) {
  return dataType = analyser.analyseLiteralExpr(this);
}

custom_ir::Value *LiteralExpr::codegen(custom_ir::IRGenerator &irGen) {
  return irGen.generateLiteral(this);
}

CastExpr::CastExpr(std::unique_ptr<Expr> expr, Type *dataType)
    : expr(std::move(expr)) {
  this->dataType = dataType;
}

Type *CastExpr::analyse(SemanticAnalyser &analyser) { return dataType; }

custom_ir::Value *CastExpr::codegen(custom_ir::IRGenerator &irGen) {
  return irGen.generateCast(this);
}

VariableExpr::VariableExpr(const std::string &n) : name(n) {}

Type *VariableExpr::analyse(SemanticAnalyser &analyser) {
  return dataType = analyser.analyseVariableExpr(this);
}

custom_ir::Value *VariableExpr::codegen(custom_ir::IRGenerator &irGen) {
  return irGen.generateVariable(this);
}

custom_ir::Value *VariableExpr::codegenLValue(custom_ir::IRGenerator &irGen) {
  return irGen.generateVariableLValue(this);
}

bool VariableExpr::isLValue() { return true; }

CallExpr::CallExpr(std::unique_ptr<Expr> callee,
                   std::vector<std::unique_ptr<Expr>> args)
    : callee(std::move(callee)), arguments(std::move(args)) {}

Type *CallExpr::analyse(SemanticAnalyser &analyser) {
  return dataType = analyser.analyseFunctionCall(this);
}

custom_ir::Value *CallExpr::codegen(custom_ir::IRGenerator &irGen) {
  return irGen.generateFunctionCall(this);
}

UnaryExpr::UnaryExpr(TOKEN_TYPE op, std::unique_ptr<Expr> operand)
    : op(op), operand(std::move(operand)) {}

Type *UnaryExpr::analyse(SemanticAnalyser &analyser) {
  return dataType = analyser.analyseUnaryExpr(this);
}

custom_ir::Value *UnaryExpr::codegen(custom_ir::IRGenerator &irGen) {
  return irGen.generateUnaryExpr(this);
}

custom_ir::Value *UnaryExpr::codegenLValue(custom_ir::IRGenerator &irGen) {
  return irGen.generateUnaryExprLValue(this);
}

bool UnaryExpr::isLValue() { return op == TOKEN_TYPE::ASTERISK; }

SizeofExpr::SizeofExpr(Type *type) { dataType = type; }

Type *SizeofExpr::analyse(SemanticAnalyser &analyser) {
  return dataType = analyser.analyseSizeofExpr(this);
}

custom_ir::Value *SizeofExpr::codegen(custom_ir::IRGenerator &irGen) {
  return irGen.generateSizeofExpr(this);
}
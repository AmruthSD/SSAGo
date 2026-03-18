#include <AST.hpp>
#include <IRGenerator.hpp>
#include <SemanticAnalyser.hpp>

BinaryExpr::BinaryExpr(TOKEN_TYPE oper, std::unique_ptr<Expr> lhs,
                       std::unique_ptr<Expr> rhs)
    : op(oper), left(std::move(lhs)), right(std::move(rhs)) {}

Type *BinaryExpr::analyse(SemanticAnalyser &analyser) {
  return dataType = analyser.analyseBinaryExpr(this);
}

llvm::Value *BinaryExpr::codegen(IRGenerator &irGen) {
  return irGen.generateBinary(this);
}

LiteralExpr::LiteralExpr(const std::string &val, Type *dataType) : value(val) {
  this->dataType = dataType;
}

Type *LiteralExpr::analyse(SemanticAnalyser &analyser) {
  return dataType = analyser.analyseLiteralExpr(this);
}

llvm::Value *LiteralExpr::codegen(IRGenerator &irGen) {
  return irGen.generateLiteral(this);
}

CastExpr::CastExpr(std::unique_ptr<Expr> expr, Type *dataType)
    : expr(std::move(expr)) {
  this->dataType = dataType;
}

Type *CastExpr::analyse(SemanticAnalyser &analyser) { return dataType; }

llvm::Value *CastExpr::codegen(IRGenerator &irGen) {
  return irGen.generateCast(this);
}

VariableExpr::VariableExpr(const std::string &n) : name(n) {}

Type *VariableExpr::analyse(SemanticAnalyser &analyser) {
  return dataType = analyser.analyseVariableExpr(this);
}

llvm::Value *VariableExpr::codegen(IRGenerator &irGen) {
  return irGen.generateVariable(this);
}

llvm::Value *VariableExpr::codegenLValue(IRGenerator &irGen) {
  return irGen.generateVariableLValue(this);
}

bool VariableExpr::isLValue() { return true; }

CallExpr::CallExpr(std::unique_ptr<Expr> callee,
                   std::vector<std::unique_ptr<Expr>> args)
    : callee(std::move(callee)), arguments(std::move(args)) {}

Type *CallExpr::analyse(SemanticAnalyser &analyser) {
  return dataType = analyser.analyseFunctionCall(this);
}

llvm::Value *CallExpr::codegen(IRGenerator &irGen) {
  return irGen.generateFunctionCall(this);
}

UnaryExpr::UnaryExpr(TOKEN_TYPE op, std::unique_ptr<Expr> operand)
    : op(op), operand(std::move(operand)) {}

Type *UnaryExpr::analyse(SemanticAnalyser &analyser) {
  return dataType = analyser.analyseUnaryExpr(this);
}

llvm::Value *UnaryExpr::codegen(IRGenerator &irGen) {
  return irGen.generateUnaryExpr(this);
}

llvm::Value *UnaryExpr::codegenLValue(IRGenerator &irGen) {
  return irGen.generateUnaryExprLValue(this);
}

bool UnaryExpr::isLValue() { return op == TOKEN_TYPE::ASTERISK; }

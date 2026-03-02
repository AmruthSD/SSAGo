#include <AST.hpp>
#include <IRGenerator.hpp>
#include <SemanticAnalyser.hpp>

BinaryExpr::BinaryExpr(TOKEN_TYPE oper, std::unique_ptr<Expr> lhs,
                       std::unique_ptr<Expr> rhs)
    : op(oper), left(std::move(lhs)), right(std::move(rhs)) {}

DATA_TYPE BinaryExpr::analyse(SemanticAnalyser &analyser) {
  return dataType = analyser.analyseBinaryExpr(this);
}

llvm::Value *BinaryExpr::codegen(IRGenerator &irGen) {
  return irGen.generateBinary(this);
}

LiteralExpr::LiteralExpr(const std::string &val, DATA_TYPE dataType)
    : value(val) {
  this->dataType = dataType;
}

DATA_TYPE LiteralExpr::analyse(SemanticAnalyser &analyser) {
  return dataType = analyser.analyseLiteralExpr(this);
}

llvm::Value *LiteralExpr::codegen(IRGenerator &irGen) {
  return irGen.generateLiteral(this);
}

CastExpr::CastExpr(std::unique_ptr<Expr> expr, DATA_TYPE dataType)
    : expr(std::move(expr)) {
  this->dataType = dataType;
}

DATA_TYPE CastExpr::analyse(SemanticAnalyser &analyser) { return dataType; }

llvm::Value *CastExpr::codegen(IRGenerator &irGen) {
  return irGen.generateCast(this);
}

VariableExpr::VariableExpr(const std::string &n) : name(n) {}

DATA_TYPE VariableExpr::analyse(SemanticAnalyser &analyser) {
  return dataType = analyser.analyseVariableExpr(this);
}

llvm::Value *VariableExpr::codegen(IRGenerator &irGen) {
  return irGen.generateVariable(this);
}

CallExpr::CallExpr(std::unique_ptr<Expr> callee,
                   std::vector<std::unique_ptr<Expr>> args)
    : callee(std::move(callee)), arguments(std::move(args)) {}

DATA_TYPE CallExpr::analyse(SemanticAnalyser &analyser) {
  return dataType = analyser.analyseFunctionCall(this);
}

llvm::Value *CallExpr::codegen(IRGenerator &irGen) {
  return irGen.generateFunctionCall(this);
}

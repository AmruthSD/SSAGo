#include <AST.hpp>
#include <IRGenerator.hpp>
#include <SemanticAnalyser.hpp>

Program::Program() = default;

Program::Program(std::vector<std::unique_ptr<Statement>> stmts)
    : statements(std::move(stmts)) {}

void Program::analyse(SemanticAnalyser &analyser) {
  analyser.analyseProgram(this);
}

llvm::Value *Program::codegen(IRGenerator &irGen) {
  return irGen.generateProgram(this);
}

ExpressionStmt::ExpressionStmt(std::unique_ptr<Expr> expr)
    : expression(std::move(expr)) {}

void ExpressionStmt::analyse(SemanticAnalyser &analyser) {
  analyser.analyseExpressionStmt(this);
}

llvm::Value *ExpressionStmt::codegen(IRGenerator &irGen) {
  return irGen.generateExpressionStmt(this);
}

BinaryExpr::BinaryExpr(TOKEN_TYPE oper, std::unique_ptr<Expr> lhs,
                       std::unique_ptr<Expr> rhs)
    : op(oper), left(std::move(lhs)), right(std::move(rhs)) {}

DATA_TYPE BinaryExpr::analyse(SemanticAnalyser &analyser) {
  return analyser.analyseBinaryExpr(this);
}

llvm::Value *BinaryExpr::codegen(IRGenerator &irGen) {
  return irGen.generateBinary(this);
}

NumberExpr::NumberExpr(const std::string &val) : value(val) {}

DATA_TYPE NumberExpr::analyse(SemanticAnalyser &analyser) {
  return analyser.analyseNumberExpr(this);
}

llvm::Value *NumberExpr::codegen(IRGenerator &irGen) {
  return irGen.generateNumber(this);
}

VariableExpr::VariableExpr(const std::string &n) : name(n) {}

DATA_TYPE VariableExpr::analyse(SemanticAnalyser &analyser) {
  return analyser.analyseVariableExpr(this);
}

llvm::Value *VariableExpr::codegen(IRGenerator &irGen) {
  return irGen.generateVariable(this);
}

DeclarationStmt::DeclarationStmt(std::string identifier, DATA_TYPE dataType,
                                 std::unique_ptr<Expr> expr)
    : identifier(std::move(identifier)), dataType(dataType),
      expr(std::move(expr)) {}

void DeclarationStmt::analyse(SemanticAnalyser &analyser) {
  analyser.analyseDeclarationStmt(this);
}

llvm::Value *DeclarationStmt::codegen(IRGenerator &irGen) {
  return irGen.generateDeclaration(this);
}

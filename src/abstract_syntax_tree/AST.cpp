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

LiteralExpr::LiteralExpr(const std::string &val, DATA_TYPE dataType)
    : value(val) {
  this->dataType = dataType;
}

DATA_TYPE LiteralExpr::analyse(SemanticAnalyser &analyser) {
  return analyser.analyseLiteralExpr(this);
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

FunctionStmt::FunctionStmt(std::string identifier, DATA_TYPE dataType,
                           std::unique_ptr<Program> prog,
                           std::vector<std::pair<std::string, DATA_TYPE>> args)
    : identifier(identifier), prgm(std::move(prog)), arguments(std::move(args)),
      dataType(dataType) {}

void FunctionStmt::analyse(SemanticAnalyser &analyser) {
  analyser.analyseFunctionStmt(this);
}

llvm::Value *FunctionStmt::codegen(IRGenerator &irGen) {
  return irGen.generateFunction(this);
}
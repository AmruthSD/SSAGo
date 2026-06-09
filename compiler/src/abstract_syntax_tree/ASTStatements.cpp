#include <AST.hpp>
#include <CustomIRGenerator.hpp>
#include <SemanticAnalyser.hpp>

ExpressionStmt::ExpressionStmt(std::unique_ptr<Expr> expr)
    : expression(std::move(expr)) {}

void ExpressionStmt::analyse(SemanticAnalyser &analyser) {
  analyser.analyseExpressionStmt(this);
}

custom_ir::Value *ExpressionStmt::codegen(custom_ir::IRGenerator &irGen) {
  return irGen.generateExpressionStmt(this);
}

DeclarationStmt::DeclarationStmt(std::string identifier, Type *dataType,
                                 std::unique_ptr<Expr> expr)
    : identifier(std::move(identifier)), dataType(std::move(dataType)),
      expr(std::move(expr)) {}

void DeclarationStmt::analyse(SemanticAnalyser &analyser) {
  analyser.analyseDeclarationStmt(this);
}

custom_ir::Value *DeclarationStmt::codegen(custom_ir::IRGenerator &irGen) {
  return irGen.generateDeclaration(this);
}

FunctionStmt::FunctionStmt(std::string identifier, Type *dataType,
                           std::unique_ptr<BlockStmt> body,
                           std::vector<std::pair<std::string, Type *>> args)
    : identifier(identifier), body(std::move(body)), arguments(std::move(args)),
      dataType(dataType) {}

void FunctionStmt::analyse(SemanticAnalyser &analyser) {
  analyser.analyseFunctionStmt(this);
}

custom_ir::Value *FunctionStmt::codegen(custom_ir::IRGenerator &irGen) {
  return irGen.generateFunction(this);
}

BlockStmt::BlockStmt(std::vector<std::unique_ptr<Statement>> body)
    : body(std::move(body)) {}

void BlockStmt::analyse(SemanticAnalyser &analyser) {
  analyser.analyseBlock(this);
}

custom_ir::Value *BlockStmt::codegen(custom_ir::IRGenerator &irGen) {
  return irGen.generateBlock(this);
}

ReturnStmt::ReturnStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}

void ReturnStmt::analyse(SemanticAnalyser &analyser) {
  analyser.analyseReturn(this);
}

custom_ir::Value *ReturnStmt::codegen(custom_ir::IRGenerator &irGen) {
  return irGen.generateReturn(this);
}

IfStmt::IfStmt(std::unique_ptr<Expr> condition,
               std::unique_ptr<Statement> thenBranch,
               std::unique_ptr<Statement> elseBranch)
    : condition(std::move(condition)), thenBranch(std::move(thenBranch)),
      elseBranch(std::move(elseBranch)) {}

void IfStmt::analyse(SemanticAnalyser &analyser) {
  analyser.analyseIfElse(this);
}

custom_ir::Value *IfStmt::codegen(custom_ir::IRGenerator &irGen) {
  return irGen.generateIfElse(this);
}

WhileStmt::WhileStmt(std::unique_ptr<Expr> condition,
                     std::unique_ptr<Statement> thenBranch)
    : condition(std::move(condition)), thenBranch(std::move(thenBranch)) {}

void WhileStmt::analyse(SemanticAnalyser &analyser) {
  analyser.analyseWhile(this);
}

custom_ir::Value *WhileStmt::codegen(custom_ir::IRGenerator &irGen) {
  return irGen.generateWhile(this);
}

void BreakStmt::analyse(SemanticAnalyser &analyser) {
  analyser.analyseBreak(this);
}

custom_ir::Value *BreakStmt::codegen(custom_ir::IRGenerator &irGen) {
  return irGen.generateBreak(this);
}
void ContinueStmt::analyse(SemanticAnalyser &analyser) {
  analyser.analyseContinue(this);
}

custom_ir::Value *ContinueStmt::codegen(custom_ir::IRGenerator &irGen) {
  return irGen.generateContinue(this);
}

GoStmt::GoStmt(std::string callee, std::vector<std::unique_ptr<Expr>> args)
    : callee(std::move(callee)), arguments(std::move(args)) {}

void GoStmt::analyse(SemanticAnalyser &analyser) {
  analyser.analyseGoFunc(this);
}

custom_ir::Value *GoStmt::codegen(custom_ir::IRGenerator &irGen) {
  return irGen.generateGoFunc(this);
}
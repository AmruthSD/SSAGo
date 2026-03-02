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
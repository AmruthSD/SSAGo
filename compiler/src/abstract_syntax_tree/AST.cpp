#include <AST.hpp>
#include <CustomIRGenerator.hpp>
#include <SemanticAnalyser.hpp>

Program::Program() = default;

Program::Program(std::vector<std::unique_ptr<Statement>> stmts)
    : statements(std::move(stmts)) {}

void Program::analyse(SemanticAnalyser &analyser) {
  analyser.analyseProgram(this);
}

custom_ir::Value *Program::codegen(custom_ir::IRGenerator &irGen) {
  return irGen.generateProgram(this);
}
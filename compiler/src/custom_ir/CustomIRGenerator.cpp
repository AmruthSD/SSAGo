#include <CustomIRGenerator.hpp>

custom_ir::IRGenerator::IRGenerator(SemanticAnalyser &semanticAnalyser)
    : semanticAnalyser(semanticAnalyser) {
  module = new ModuleIR();
  semanticAnalyser.ast.get()->codegen(*this);
}
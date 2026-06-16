#include <CustomIRBuilder.hpp>
#include <CustomIRGenerator.hpp>
#include <CustomIRPrinter.hpp>
#include <Lexer.hpp>
#include <Parser.hpp>
#include <SemanticAnalyser.hpp>
#include <fstream>
#include <iostream>
#include <llvm/Support/raw_ostream.h>
#include <stdexcept>

int main(int argc, char **argv) {

  if (argc <= 2) {
    std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>\n";
    return 1;
  }

  std::ifstream input(argv[1]);

  if (!input.is_open()) {
    std::cerr << "Error: Could not open file '" << argv[1] << "'\n";
    return 1;
  }

  std::cout << "File opened successfully.\n";

  Lexer lexer(input);
  Parser parser(lexer);
  SemanticAnalyser semanticAnalyser(parser);
  // IRGenerator irGen(semanticAnalyser);
  // irGen.optimizeModule();
  // irGen.writeIRToFile(argv[2]);
  // std::cout << "IR written to the output.ll\n";
  custom_ir::IRPrinter printer;
  custom_ir::IRBuilder builder;
  custom_ir::IRGenerator customIRGenerator(semanticAnalyser, builder, printer);

  return 0;
}
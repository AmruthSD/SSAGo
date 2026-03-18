#include <IRGenerator.hpp>
#include <Lexer.hpp>
#include <Parser.hpp>
#include <SemanticAnalyser.hpp>
#include <fstream>
#include <iostream>
#include <llvm/Support/raw_ostream.h>
#include <stdexcept>

int main(int argc, char **argv) {

  if (argc <= 1) {
    std::cerr << "Usage: " << argv[0] << " <input_file>\n";
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
  IRGenerator irGen(semanticAnalyser);
  irGen.optimizeModule();
  irGen.writeIRToFile("output.ll");
  std::cout << "IR written to the output.ll\n";
  return 0;
}
#include <IRGenerator.hpp>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

void IRGenerator::writeIRToFile(const std::string &filename) {
  std::error_code EC;
  llvm::raw_fd_ostream outFile(filename, EC);

  if (EC) {
    throw std::runtime_error("Could not open file: " + filename);
  }

  module->print(outFile, nullptr);
}
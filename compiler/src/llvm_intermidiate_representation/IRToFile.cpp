#include <LLVMIRGenerator.hpp>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

namespace custom_ir {
void LLVMIRGenerator::writeIRToFile(const std::string &filename) {
  std::error_code EC;
  llvm::raw_fd_ostream outFile(filename, EC);

  if (EC) {
    throw std::runtime_error("Could not open file: " + filename);
  }

  module->print(outFile, nullptr);
}
} // namespace custom_ir
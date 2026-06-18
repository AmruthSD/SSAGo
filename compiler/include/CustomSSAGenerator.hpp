#pragma once

#include <CustomIR.hpp>
#include <CustomIRPrinter.hpp>
#include <DominanceAnalysis.hpp>
#include <stack>
#include <unordered_map>
#include <unordered_set>

namespace custom_ir {
class SSAGenerator {
  DominatorAnalysis &dominanceAnalysis;
  IRPrinter &printer;

  std::unordered_map<int, std::unordered_set<BasicBlockIR *>> definitionBlocks;
  std::unordered_map<int, int> nextVersion;

  std::unordered_map<int, std::stack<VariableValue *>> currentVersion;

public:
  ModuleIR *module;
  SSAGenerator(ModuleIR *module, DominatorAnalysis &dominanceAnalysis,
               IRPrinter &printer);

  void getDefinitionBlocks();
  void insertPhiNodes();
  void renameModule();
  void renameBlock(BasicBlockIR *block);
  void renameGlobalDefinitions(BasicBlockIR *block);
};
} // namespace custom_ir

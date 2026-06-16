#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace custom_ir {
class ModuleIR;
class BasicBlockIR;
class Function;

struct DominatorInfo {
  std::unordered_set<BasicBlockIR *> dominators;

  BasicBlockIR *idom = nullptr;

  std::vector<BasicBlockIR *> domChildren;

  std::unordered_set<BasicBlockIR *> dominanceFrontier;
};

class DominatorAnalysis {
public:
  explicit DominatorAnalysis(ModuleIR *module);

  void run();

  DominatorInfo &getInfo(BasicBlockIR *block);

private:
  ModuleIR *module;

  std::unordered_map<BasicBlockIR *, DominatorInfo> info;

  void computeDominators(Function *func);

  void computeImmediateDominators(Function *func);

  void buildDominatorTree(Function *func);

  void computeDominanceFrontiers(Function *func);
};

} // namespace custom_ir
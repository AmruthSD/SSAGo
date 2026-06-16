#include <CustomIR.hpp>
#include <DominanceAnalysis.hpp>

namespace custom_ir {
DominatorAnalysis::DominatorAnalysis(ModuleIR *module) : module(module) {}

DominatorInfo &DominatorAnalysis::getInfo(BasicBlockIR *block) {
  return info[block];
}

void DominatorAnalysis::run() {
  for (auto &[_, func] : module->functions)
    computeDominators(func);

  for (auto &[_, func] : module->functions)
    computeImmediateDominators(func);

  for (auto &[_, func] : module->functions)
    buildDominatorTree(func);

  for (auto &[_, func] : module->functions)
    computeDominanceFrontiers(func);
}

std::unordered_set<BasicBlockIR *>
intersect(const std::unordered_set<BasicBlockIR *> &a,
          const std::unordered_set<BasicBlockIR *> &b) {

  std::unordered_set<BasicBlockIR *> result;

  for (auto *block : a) {
    if (b.find(block) != b.end())
      result.insert(block);
  }

  return result;
}

void DominatorAnalysis::computeDominators(Function *func) {
  auto &blocks = func->blocks;

  BasicBlockIR *entry = blocks[0];

  for (auto *block : blocks) {
    if (block == entry) {
      info[block].dominators = {entry};
    } else {
      for (auto *other : blocks)
        info[block].dominators.insert(other);
    }
  }

  bool changed = true;

  while (changed) {
    changed = false;

    for (auto *block : blocks) {

      if (block == entry)
        continue;

      auto newDom = info[block->predecessor[0]].dominators;
      for (size_t i = 1; i < block->predecessor.size(); i++) {
        newDom = intersect(newDom, info[block->predecessor[i]].dominators);
      }
      newDom.insert(block);

      if (newDom != info[block].dominators) {
        info[block].dominators = std::move(newDom);
        changed = true;
      }
    }
  }
}

void DominatorAnalysis::computeImmediateDominators(Function *func) {

  auto &blocks = func->blocks;
  BasicBlockIR *entry = blocks[0];

  info[entry].idom = nullptr;

  for (auto *block : blocks) {

    if (block == entry)
      continue;

    auto strictDom = info[block].dominators;
    strictDom.erase(block);

    BasicBlockIR *idom = nullptr;

    for (auto *candidate : strictDom) {

      bool isImmediate = true;

      for (auto *other : strictDom) {

        if (candidate == other)
          continue;

        if (info[candidate].dominators.count(other)) {
          isImmediate = false;
          break;
        }
      }

      if (isImmediate) {
        idom = candidate;
        break;
      }
    }

    info[block].idom = idom;
  }
}

void DominatorAnalysis::buildDominatorTree(Function *func) {

  for (auto *block : func->blocks)
    info[block].domChildren.clear();

  for (auto *block : func->blocks) {

    BasicBlockIR *idom = info[block].idom;

    if (idom)
      info[idom].domChildren.push_back(block);
  }
}

void DominatorAnalysis::computeDominanceFrontiers(Function *func) {

  for (auto *block : func->blocks)
    info[block].dominanceFrontier.clear();

  for (auto *block : func->blocks) {

    if (block->predecessor.size() < 2)
      continue;

    for (auto *pred : block->predecessor) {

      BasicBlockIR *runner = pred;

      while (runner != info[block].idom) {

        info[runner].dominanceFrontier.insert(block);

        runner = info[runner].idom;
      }
    }
  }
}

} // namespace custom_ir

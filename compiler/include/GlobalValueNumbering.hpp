#pragma once

#include <CustomIR.hpp>
#include <DominanceAnalysis.hpp>
#include <unordered_map>

namespace custom_ir {

struct KeyValue {
  Opcode opcode;
  std::string lhs;
  std::string rhs;

  bool operator==(const KeyValue &other) const {
    return opcode == other.opcode && lhs == other.lhs && rhs == other.rhs;
  }
};

struct KeyValueHash {
  size_t operator()(const KeyValue &key) const {
    size_t h1 = std::hash<int>()(static_cast<int>(key.opcode));
    size_t h2 = std::hash<std::string>()(key.lhs);
    size_t h3 = std::hash<std::string>()(key.rhs);

    return h1 ^ (h2 << 1) ^ (h3 << 2);
  }
};

class GlobalValueNumbering {
  ModuleIR *module;
  DominatorAnalysis *domAnalysis;

  std::unordered_map<KeyValue, Value *, KeyValueHash> hashValue;
  std::map<std::string, ValueUse *> valueUses;

public:
  void GVN();
  void DFS(BasicBlockIR *block);
  bool isCommutative(Opcode opcode);
  void FillValueUses();
  ValueUse *getOrCreateUses(std::string);
  void replaceAllUsesWith(Value *oldValue, Value *newValue);

  GlobalValueNumbering(ModuleIR *module, DominatorAnalysis *domAnalysis)
      : module(module), domAnalysis(domAnalysis) {
    this->GVN();
  }
};

} // namespace custom_ir

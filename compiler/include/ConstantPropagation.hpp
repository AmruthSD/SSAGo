#pragma once

#include <CustomIR.hpp>

namespace custom_ir {
class ConstantPropagation {
  ModuleIR *module;
  std::map<std::string, ValueUse *> valueUses;
  std::map<std::string, Constant *> valueConst;

public:
  void FillValueUses();
  bool tryFold(Instruction *inst);
  void replaceAllUsesWith(Value *newValue, Value *oldValue);
  ValueUse *getOrCreateUses(std::string);
  void constantPropagateModule();
  ConstantPropagation(ModuleIR *module) : module(module) {
    this->constantPropagateModule();
  }
};
} // namespace custom_ir
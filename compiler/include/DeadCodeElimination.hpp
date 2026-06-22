#pragma once

#include <ConstantPropagation.hpp>
#include <CustomIR.hpp>

namespace custom_ir {
class DeadCodeElimination {
  ModuleIR *module;
  std::map<std::string, ValueUse *> valueUses;

public:
  void FillValueUses();
  ValueUse *getOrCreateUses(std::string);
  void deadCodeEliminateValues();
  bool tryDelete(Instruction *);
  DeadCodeElimination(ModuleIR *module) : module(module) {
    this->deadCodeEliminateValues();
  }
};
} // namespace custom_ir

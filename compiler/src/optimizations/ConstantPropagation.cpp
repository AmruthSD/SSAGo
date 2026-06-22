#include <ConstantPropagation.hpp>
#include <map>
#include <set>

namespace custom_ir {
inline int64_t asInt(const std::string &value) { return std::stoll(value); }

inline double asFloat(const std::string &value) { return std::stod(value); }

ValueUse *ConstantPropagation::getOrCreateUses(std::string name) {
  if (valueUses.find(name) == valueUses.end())
    return valueUses[name] = new ValueUse();

  return valueUses[name];
}

void ConstantPropagation::FillValueUses() {
  for (auto &[_, func] : module->functions) {
    for (auto &block : func->blocks) {
      for (auto &inst : block->instructions) {
        if (inst->opcode == Opcode::Store) {
          Value *var = inst->operands[1], *storingVal = inst->operands[0];
          VariableValue *variable = dynamic_cast<VariableValue *>(var);
          if (variable != nullptr) {
            ValueUse *useVariable = getOrCreateUses(
                variable->value + std::to_string(variable->version));
            useVariable->definition = inst;
          }
          Constant *constVal = dynamic_cast<Constant *>(storingVal);
          if (constVal == nullptr) {
            ValueUse *useValue = getOrCreateUses(storingVal->value);
            useValue->uses.push_back(inst);
          }
        }
        if (inst->opcode == Opcode::Load) {
          Value *val = inst->operands[0], *res = inst->operands[1];
          VariableValue *variable = dynamic_cast<VariableValue *>(val);
          if (variable != nullptr) {
            ValueUse *useVariable = getOrCreateUses(
                variable->value + std::to_string(variable->version));
            useVariable->uses.push_back(inst);
          } else {
            ValueUse *useVariable = getOrCreateUses(val->value);
            useVariable->uses.push_back(inst);
          }
          ValueUse *resUse = getOrCreateUses(res->value);
          resUse->definition = inst;
        }
        if (inst->opcode == Opcode::Call ||
            inst->opcode == Opcode::CallExternal ||
            inst->opcode == Opcode::GoCall) {
          if (inst->value == "scanf") {
            for (int i = 0; i < inst->operands.size() - 1; i++) {
              VariableValue *var =
                  dynamic_cast<VariableValue *>(inst->operands[i]);
              if (var != nullptr) {
                ValueUse *use =
                    getOrCreateUses(var->value + std::to_string(var->version));
                use->definition = inst;
              }
            }
          } else {
            for (int i = 0; i < inst->operands.size() - 1; i++) {
              VariableValue *var =
                  dynamic_cast<VariableValue *>(inst->operands[i]);
              TempValue *tempVal = dynamic_cast<TempValue *>(inst->operands[i]);
              if (var != nullptr) {
                ValueUse *use =
                    getOrCreateUses(var->value + std::to_string(var->version));
                use->uses.push_back(inst);
              }
              if (tempVal != nullptr) {
                ValueUse *use = getOrCreateUses(tempVal->value);
                use->uses.push_back(inst);
              }
            }
          }
        }
        if (inst->opcode == Opcode::Branch ||
            inst->opcode == Opcode::CondBranch ||
            inst->opcode == Opcode::Alloca || inst->opcode == Opcode::Bit_Cast)
          continue;
        if (inst->opcode == Opcode::Cast_FLOAT ||
            inst->opcode == Opcode::Cast_INT) {
          TempValue *res = dynamic_cast<TempValue *>(inst->operands[1]),
                    *tempVal = dynamic_cast<TempValue *>(inst->operands[0]);
          VariableValue *val = dynamic_cast<VariableValue *>(inst->operands[0]);
          if (val != nullptr) {
            ValueUse *use =
                getOrCreateUses(val->value + std::to_string(val->version));
            use->uses.push_back(inst);
          }
          if (tempVal != nullptr) {
            ValueUse *use = getOrCreateUses(tempVal->value);
            use->uses.push_back(inst);
          }
          ValueUse *use = getOrCreateUses(res->value);
          use->definition = (inst);
        }
        if (inst->opcode == Opcode::Phi) {
          for (int i = 0; i < inst->operands.size(); i++) {
            if (i == 0) {
              VariableValue *var =
                  dynamic_cast<VariableValue *>(inst->operands[0]);
              ValueUse *use =
                  getOrCreateUses(var->value + std::to_string(var->version));
              use->definition = inst;
            } else {
              VariableValue *var =
                  dynamic_cast<VariableValue *>(inst->operands[i]);
              ValueUse *use =
                  getOrCreateUses(var->value + std::to_string(var->version));
              use->uses.push_back(inst);
            }
          }
        } else {
          switch (inst->opcode) {
          case Opcode::Add:
          case Opcode::Sub:
          case Opcode::Mul:
          case Opcode::Div:

          case Opcode::FAdd:
          case Opcode::FSub:
          case Opcode::FMul:
          case Opcode::FDiv:

          case Opcode::ICmpEQ:
          case Opcode::ICmpNE:
          case Opcode::ICmpLT:
          case Opcode::ICmpGT:

          case Opcode::FCmpEQ:
          case Opcode::FCmpNE:
          case Opcode::FCmpLT:
          case Opcode::FCmpGT:

          case Opcode::And:
          case Opcode::Or:
            break;

          default:
            continue;
          }

          TempValue *res = dynamic_cast<TempValue *>(inst->operands[2]),
                    *val1 = dynamic_cast<TempValue *>(inst->operands[0]),
                    *val2 = dynamic_cast<TempValue *>(inst->operands[1]);

          Constant *constVal1 = dynamic_cast<Constant *>(inst->operands[0]),
                   *constVal2 = dynamic_cast<Constant *>(inst->operands[1]);

          ValueUse *resUse = getOrCreateUses(res->value);
          resUse->definition = inst;

          if (val1 != nullptr) {
            ValueUse *use = getOrCreateUses(val1->value);
            use->uses.push_back(inst);
          }

          if (val2 != nullptr) {
            ValueUse *use = getOrCreateUses(val2->value);
            use->uses.push_back(inst);
          }
        }
      }
    }
  }
}

void ConstantPropagation::replaceAllUsesWith(Value *oldValue, Value *newValue) {

  VariableValue *var = dynamic_cast<VariableValue *>(oldValue);
  TempValue *tempVar = dynamic_cast<TempValue *>(oldValue);

  if (var != nullptr)
    return;

  if (tempVar == nullptr)
    return;
  ValueUse *currentUsers = getOrCreateUses(tempVar->value);
  for (auto *inst : currentUsers->uses) {

    for (auto &operand : inst->operands) {

      if (operand == oldValue)
        operand = newValue;
    }
  }
}

bool ConstantPropagation::tryFold(Instruction *inst) {
  if (inst->opcode == Opcode::Store) {
    Value *var = inst->operands[1], *storingVal = inst->operands[0];
    VariableValue *variable = dynamic_cast<VariableValue *>(var);
    if (variable == nullptr) {
      return false;
    }
    Constant *constVal = dynamic_cast<Constant *>(storingVal);
    if (constVal != nullptr) {
      valueConst[variable->value + std::to_string(variable->version)] =
          constVal;
      replaceAllUsesWith(var, storingVal);
      return true;
    }
  }
  if (inst->opcode == Opcode::Load) {
    Value *val = inst->operands[0], *res = inst->operands[1];
    VariableValue *variable = dynamic_cast<VariableValue *>(val);
    if (variable != nullptr) {
      std::string name = variable->value + std::to_string(variable->version);
      if (valueConst.find(name) != valueConst.end()) {
        Constant *newValue = valueConst[name];
        replaceAllUsesWith(res, newValue);
        return true;
      }
    }
  }
  if (inst->opcode == Opcode::Call || inst->opcode == Opcode::CallExternal ||
      inst->opcode == Opcode::GoCall) {
    return false;
  }
  if (inst->opcode == Opcode::Branch || inst->opcode == Opcode::CondBranch ||
      inst->opcode == Opcode::Alloca || inst->opcode == Opcode::Bit_Cast)
    return false;
  if (inst->opcode == Opcode::Cast_FLOAT || inst->opcode == Opcode::Cast_INT) {
    TempValue *res = dynamic_cast<TempValue *>(inst->operands[1]),
              *tempVal = dynamic_cast<TempValue *>(inst->operands[0]);
    if (tempVal != nullptr) {
      std::string name = tempVal->value;
      if (valueConst.find(name) != valueConst.end()) {
        Constant *precastValue = valueConst[name];
        Constant *newValue;
        if (inst->opcode == Opcode::Cast_FLOAT)
          newValue = new Constant(res->dataType,
                                  std::to_string(asFloat(precastValue->value)));

        if (inst->opcode == Opcode::Cast_INT)
          newValue = new Constant(res->dataType,
                                  std::to_string(asInt(precastValue->value)));
        replaceAllUsesWith(res, newValue);
        return true;
      }
    }
  }
  if (inst->opcode == Opcode::Phi) {
    return false;
  } else {
    TempValue *res = dynamic_cast<TempValue *>(inst->operands[2]);
    Constant *constVal1 = dynamic_cast<Constant *>(inst->operands[0]),
             *constVal2 = dynamic_cast<Constant *>(inst->operands[1]);

    if (constVal1 == nullptr || constVal2 == nullptr)
      return false;

    Type *resultType = res->dataType;
    Constant *newValue;

    switch (inst->opcode) {

    case Opcode::Add:
      newValue =
          new Constant(resultType, std::to_string(asInt(constVal1->value) +
                                                  asInt(constVal2->value)));
      break;

    case Opcode::Sub:
      newValue =
          new Constant(resultType, std::to_string(asInt(constVal1->value) -
                                                  asInt(constVal2->value)));
      break;

    case Opcode::Mul:
      newValue =
          new Constant(resultType, std::to_string(asInt(constVal1->value) *
                                                  asInt(constVal2->value)));
      break;

    case Opcode::Div:
      if (asInt(constVal2->value) == 0)
        return false;

      newValue =
          new Constant(resultType, std::to_string(asInt(constVal1->value) /
                                                  asInt(constVal2->value)));
      break;

    case Opcode::FAdd:
      newValue =
          new Constant(resultType, std::to_string(asFloat(constVal1->value) +
                                                  asFloat(constVal2->value)));
      break;

    case Opcode::FSub:
      newValue =
          new Constant(resultType, std::to_string(asFloat(constVal1->value) -
                                                  asFloat(constVal2->value)));
      break;

    case Opcode::FMul:
      newValue =
          new Constant(resultType, std::to_string(asFloat(constVal1->value) *
                                                  asFloat(constVal2->value)));
      break;

    case Opcode::FDiv:
      if (asFloat(constVal2->value) == 0.0)
        return false;

      newValue =
          new Constant(resultType, std::to_string(asFloat(constVal1->value) /
                                                  asFloat(constVal2->value)));
      break;

    case Opcode::ICmpEQ:
      newValue =
          new Constant(resultType, std::to_string(asInt(constVal1->value) ==
                                                  asInt(constVal2->value)));
      break;

    case Opcode::ICmpNE:
      newValue =
          new Constant(resultType, std::to_string(asInt(constVal1->value) !=
                                                  asInt(constVal2->value)));
      break;

    case Opcode::ICmpLT:
      newValue =
          new Constant(resultType, std::to_string(asInt(constVal1->value) <
                                                  asInt(constVal2->value)));
      break;

    case Opcode::ICmpGT:
      newValue =
          new Constant(resultType, std::to_string(asInt(constVal1->value) >
                                                  asInt(constVal2->value)));
      break;

    case Opcode::FCmpEQ:
      newValue =
          new Constant(resultType, std::to_string(asFloat(constVal1->value) ==
                                                  asFloat(constVal2->value)));
      break;

    case Opcode::FCmpNE:
      newValue =
          new Constant(resultType, std::to_string(asFloat(constVal1->value) !=
                                                  asFloat(constVal2->value)));
      break;

    case Opcode::FCmpLT:
      newValue =
          new Constant(resultType, std::to_string(asFloat(constVal1->value) <
                                                  asFloat(constVal2->value)));
      break;

    case Opcode::FCmpGT:
      newValue =
          new Constant(resultType, std::to_string(asFloat(constVal1->value) >
                                                  asFloat(constVal2->value)));
      break;

    case Opcode::And:
      newValue =
          new Constant(resultType, std::to_string(asInt(constVal1->value) &&
                                                  asInt(constVal2->value)));
      break;

    case Opcode::Or:
      newValue =
          new Constant(resultType, std::to_string(asInt(constVal1->value) ||
                                                  asInt(constVal2->value)));
      break;

    default:
      return false;
    }
    replaceAllUsesWith(res, newValue);
    return true;
  }
  return false;
}

void ConstantPropagation::constantPropagateModule() {
  FillValueUses();
  std::set<Instruction *> foldedInst;
  bool changed = false;
  do {
    changed = false;

    for (auto &[_, func] : module->functions) {
      for (auto &block : func->blocks) {
        for (auto &inst : block->instructions) {
          if (foldedInst.find(inst) == foldedInst.end()) {
            bool res = tryFold(inst);
            changed |= res;
            if (res) {
              foldedInst.insert(inst);
            }
          }
        }
      }
    }

  } while (changed == true);
}
} // namespace custom_ir
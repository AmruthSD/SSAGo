#include <GlobalValueNumbering.hpp>

namespace custom_ir {
bool GlobalValueNumbering::isCommutative(Opcode opcode) {
  switch (opcode) {

  case Opcode::Add:
  case Opcode::Mul:
  case Opcode::FAdd:
  case Opcode::FMul:
  case Opcode::And:
  case Opcode::Or:
  case Opcode::ICmpEQ:
  case Opcode::ICmpNE:
  case Opcode::FCmpEQ:
  case Opcode::FCmpNE:
    return true;

  default:
    return false;
  }
}

ValueUse *GlobalValueNumbering::getOrCreateUses(std::string name) {
  if (valueUses.find(name) == valueUses.end())
    return valueUses[name] = new ValueUse();

  return valueUses[name];
}

void GlobalValueNumbering::replaceAllUsesWith(Value *oldValue,
                                              Value *newValue) {

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

void GlobalValueNumbering::FillValueUses() {
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

void GlobalValueNumbering::DFS(BasicBlockIR *block) {

  auto savedTable = hashValue;

  for (auto *inst : block->instructions) {

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
    case Opcode::Or: {

      std::string lhs = inst->operands[0]->value;
      std::string rhs = inst->operands[1]->value;

      if (isCommutative(inst->opcode) && lhs > rhs)
        std::swap(lhs, rhs);

      KeyValue key{
          inst->opcode,
          lhs,
          rhs,
      };

      auto it = hashValue.find(key);

      TempValue *res = dynamic_cast<TempValue *>(inst->operands[2]);

      if (it != hashValue.end()) {
        replaceAllUsesWith(res, it->second);
      } else {
        hashValue[key] = res;
      }

      break;
    }

    case Opcode::Load: {
      VariableValue *var = dynamic_cast<VariableValue *>(inst->operands[0]);
      if (var == nullptr)
        break;

      KeyValue key{
          inst->opcode,
          var->value + std::to_string(var->version),
          "",
      };

      auto it = hashValue.find(key);

      TempValue *res = dynamic_cast<TempValue *>(inst->operands[1]);

      if (it != hashValue.end()) {
        replaceAllUsesWith(res, it->second);
      } else {
        hashValue[key] = res;
      }

      break;
    }

    default:
      break;
    }
  }

  for (auto *child : domAnalysis->getInfo(block).domChildren)
    DFS(child);

  hashValue = savedTable;
}

void GlobalValueNumbering::GVN() {

  FillValueUses();
  for (auto &[_, func] : module->functions) {
    hashValue.clear();
    BasicBlockIR *entry = func->blocks[0];

    DFS(entry);
  }
}
} // namespace custom_ir

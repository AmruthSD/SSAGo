#include <DeadCodeElimination.hpp>
#include <set>

namespace custom_ir {
ValueUse *DeadCodeElimination::getOrCreateUses(std::string name) {
  if (valueUses.find(name) == valueUses.end())
    return valueUses[name] = new ValueUse();

  return valueUses[name];
}

void DeadCodeElimination::FillValueUses() {
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
            inst->opcode == Opcode::Return) {
          TempValue *res = dynamic_cast<TempValue *>(inst->operands[0]);
          Constant *cst = dynamic_cast<Constant *>(inst->operands[0]);
          if (res != nullptr) {
            ValueUse *use = getOrCreateUses(res->value);
            use->uses.push_back(inst);
          }
        }
        if (inst->opcode == Opcode::Alloca || inst->opcode == Opcode::Bit_Cast)
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

bool DeadCodeElimination::tryDelete(Instruction *inst) {
  switch (inst->opcode) {
  case Opcode::Return:
  case Opcode::Call:
  case Opcode::CallExternal:
  case Opcode::GoCall:
  case Opcode::Store:
  case Opcode::Branch:
  case Opcode::CondBranch:
  case Opcode::Alloca:
    return false;
    break;

  case Opcode::Cast_FLOAT:
  case Opcode::Cast_INT: {
    TempValue *res = dynamic_cast<TempValue *>(inst->operands[1]);
    ValueUse *use = getOrCreateUses(res->value);
    if (use->uses.size() != 0)
      return false;
    return true;
  }
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
    TempValue *res = dynamic_cast<TempValue *>(inst->operands[2]);
    ValueUse *use = getOrCreateUses(res->value);
    if (use->uses.size() != 0)
      return false;
    return true;
  }
  default:
    break;
  }
  return false;
}

void DeadCodeElimination::deadCodeEliminateValues() {

  bool changed = false;
  do {
    changed = false;
    valueUses.clear();
    FillValueUses();

    for (auto &[_, func] : module->functions) {
      for (auto &block : func->blocks) {
        std::vector<Instruction *> toDelete;
        for (auto &inst : block->instructions) {
          bool res = tryDelete(inst);
          changed |= res;
          if (res) {
            toDelete.push_back(inst);
          }
        }
        for (auto &inst : toDelete) {
          auto &instructions = block->instructions;

          instructions.erase(
              std::remove(instructions.begin(), instructions.end(), inst),
              instructions.end());
        }
      }
    }

  } while (changed == true);
}

} // namespace custom_ir
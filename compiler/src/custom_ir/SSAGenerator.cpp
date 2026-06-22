#include <ConstantPropagation.hpp>
#include <CustomSSAGenerator.hpp>
#include <DeadCodeElimination.hpp>
#include <iostream>
#include <queue>

namespace custom_ir {
SSAGenerator::SSAGenerator(ModuleIR *module,
                           DominatorAnalysis &dominanceAnalysis,
                           IRPrinter &printer)
    : module(module), dominanceAnalysis(dominanceAnalysis), printer(printer) {
  dominanceAnalysis.run();
  getDefinitionBlocks();
  insertPhiNodes();
  std::cout << "phi nodes are inserted" << std::endl;
  renameModule();
  printer.print(module, "build/no_optimization.ir");
  std::cout << "constant optimization" << std::endl;
  ConstantPropagation optimizerConstant(module);
  printer.print(module, "build/const_prop.ir");
  std::cout << "dce optimization" << std::endl;
  DeadCodeElimination dce(module);
  printer.print(module, "build/postdec.ir");
}

void SSAGenerator::getDefinitionBlocks() {
  for (auto &[_, func] : module->functions) {
    for (auto *block : func->blocks) {

      for (auto *inst : block->instructions) {

        if (!(inst->opcode == Opcode::Store ||
              inst->opcode == Opcode::CallExternal))
          continue;

        if (inst->opcode == Opcode::CallExternal && inst->value == "scanf") {
          for (auto &operand : inst->operands) {

            Value *var = operand;

            VariableValue *varValue = dynamic_cast<VariableValue *>(var);
            if (varValue == nullptr)
              continue;

            definitionBlocks[varValue->variable_id].insert(block);
          }
        } else if (inst->opcode == Opcode::Store) {

          Value *var = inst->operands.back();

          VariableValue *varValue = dynamic_cast<VariableValue *>(var);
          if (varValue == nullptr)
            continue;

          definitionBlocks[varValue->variable_id].insert(block);
        }
      }
    }
  }
}

void SSAGenerator::insertPhiNodes() {

  for (auto &[_, func] : module->functions) {

    for (auto &[variableId, defBlocks] : definitionBlocks) {

      std::queue<BasicBlockIR *> worklist;

      std::unordered_set<BasicBlockIR *> hasPhi;

      for (auto *block : defBlocks)
        worklist.push(block);

      while (!worklist.empty()) {

        BasicBlockIR *X = worklist.front();
        worklist.pop();

        for (auto *Y : dominanceAnalysis.getInfo(X).dominanceFrontier) {

          if (hasPhi.find(Y) != hasPhi.end())
            continue;

          auto *phi = new PhiInstruction(
              variableId,
              new TempValue{new Type{DATA_TYPE::DATATYPE_VOID, nullptr},
                            "phinode"});

          Y->instructions.insert(Y->instructions.begin(), phi);

          hasPhi.insert(Y);

          if (defBlocks.find(Y) == defBlocks.end())
            worklist.push(Y);
        }
      }
    }
  }
}

void SSAGenerator::renameModule() {
  renameGlobalDefinitions(module->globalDeclarations);
  for (auto &[_, func] : module->functions) {
    if (func->blocks.size() > 0) {
      renameBlock(func->blocks[0]);
    }
  }
}

void SSAGenerator::renameBlock(BasicBlockIR *block) {
  std::vector<int> pushedVariables;

  for (auto *inst : block->instructions) {
    if (inst->opcode != Opcode::Phi)
      continue;

    auto *phi = static_cast<PhiInstruction *>(inst);
    int varId = phi->variableId;
    int version = nextVersion[varId]++;

    VariableValue *oldVal = variableValues[varId],
                  *newVer = new VariableValue(oldVal->dataType, oldVal->value,
                                              oldVal->variable_id);
    newVer->version = version;
    phi->operands.push_back(newVer);

    currentVersion[varId].push(newVer);
    pushedVariables.push_back(varId);
  }

  for (auto *inst : block->instructions) {
    if (inst->opcode == Opcode::Store) {

      VariableValue *variable =
          dynamic_cast<VariableValue *>(inst->operands[1]);
      if (variable == nullptr)
        continue;
      int varId = variable->variable_id;
      int version = nextVersion[varId]++;
      auto *ssaValue = new VariableValue(variable->dataType, variable->value,
                                         variable->variable_id);
      ssaValue->version = version;
      inst->operands[1] = ssaValue;
      currentVersion[varId].push(ssaValue);

      pushedVariables.push_back(varId);

      continue;
    }
    if (inst->opcode == Opcode::CallExternal && inst->value == "scanf") {
      int idx = 0;
      for (idx = 0; idx < inst->operands.size(); idx++) {

        Value *var = inst->operands[idx];

        VariableValue *variable = dynamic_cast<VariableValue *>(var);
        if (variable == nullptr)
          continue;

        int varId = variable->variable_id;
        int version = nextVersion[varId]++;
        auto *ssaValue = new VariableValue(variable->dataType, variable->value,
                                           variable->variable_id);
        ssaValue->version = version;
        inst->operands[idx] = ssaValue;
        currentVersion[varId].push(ssaValue);
        pushedVariables.push_back(varId);
      }
    }
    if (inst->opcode == Opcode::Load) {

      VariableValue *variable =
          dynamic_cast<VariableValue *>(inst->operands[0]);
      if (variable == nullptr)
        continue;
      int varId = variable->variable_id;
      if (currentVersion[varId].size() == 0) {
        std::cout << "error as the current version isnt there for " +
                         variable->value
                  << std::endl;
      }
      VariableValue *current = currentVersion[varId].top();
      inst->operands[0] = current;
      continue;
    }
    if (inst->opcode == Opcode::Alloca) {

      VariableValue *variable = static_cast<VariableValue *>(inst->operands[0]);
      int varId = variable->variable_id;
      int version = nextVersion[varId]++;
      auto *ssaValue = new VariableValue(variable->dataType, variable->value,
                                         variable->variable_id);
      ssaValue->version = version;
      inst->operands[0] = ssaValue;
      currentVersion[varId].push(ssaValue);

      pushedVariables.push_back(varId);

      continue;
    }
  }
  for (auto *succ : block->successor) {

    for (auto *inst : succ->instructions) {

      if (inst->opcode != Opcode::Phi)
        continue;

      auto *phi = static_cast<PhiInstruction *>(inst);

      int varId = phi->variableId;

      if (currentVersion[varId].empty())
        continue;

      phi->operands.push_back(currentVersion[varId].top());
    }
  }

  for (auto *child : dominanceAnalysis.getInfo(block).domChildren) {
    renameBlock(child);
  }

  for (auto it = pushedVariables.rbegin(); it != pushedVariables.rend(); ++it) {
    currentVersion[*it].pop();
  }
}

void SSAGenerator::renameGlobalDefinitions(BasicBlockIR *block) {
  for (auto *inst : block->instructions) {
    if (inst->opcode == Opcode::Global_Dec) {

      VariableValue *variable = static_cast<VariableValue *>(inst->operands[1]);
      int varId = variable->variable_id;
      int version = nextVersion[varId]++;
      auto *ssaValue = new VariableValue(variable->dataType, variable->value,
                                         variable->variable_id);
      ssaValue->version = version;
      inst->operands[1] = ssaValue;
      currentVersion[varId].push(ssaValue);

      continue;
    }
  }
}
} // namespace custom_ir

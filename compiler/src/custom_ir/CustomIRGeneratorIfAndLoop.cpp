#include <CustomIRGenerator.hpp>

custom_ir::Value *custom_ir::IRGenerator::generateIfElse(IfStmt *stmt) {
  custom_ir::Value *condValue = stmt->condition->codegen(*this);

  if (!condValue)
    throw std::runtime_error("Invalid condition in if");

  if (isNumeric(condValue->dataType)) {
    condValue = builder.CreateBinary(
        Opcode::ICmpNE, condValue,
        builder.CreateConstant(new Type{DATA_TYPE::DATATYPE_INT, nullptr}, "0"),
        "ifcond");
  } else if (condValue->dataType->base == DATA_TYPE::DATATYPE_FLOAT) {
    condValue = builder.CreateBinary(
        Opcode::FCmpNE, condValue,
        builder.CreateConstant(new Type{DATA_TYPE::DATATYPE_FLOAT, nullptr},
                               "0.0"),
        "ifcond");
  } else {
    throw std::runtime_error("Unsupported condition type in if");
  }

  custom_ir::Function *function = builder.GetInsertBlock()->parent;

  custom_ir::BasicBlockIR *thenBB = new BasicBlockIR("then", function);
  custom_ir::BasicBlockIR *elseBB = new BasicBlockIR("else", function);
  custom_ir::BasicBlockIR *mergeBB = new BasicBlockIR("ifcont", function);
  builder.CreateCondBr(condValue, thenBB, elseBB);

  builder.setInsertPoint(thenBB);
  stmt->thenBranch->codegen(*this);
  if (builder.GetInsertBlock()->instructions.size() == 0 ||
      !(builder.GetInsertBlock()->instructions.back()->opcode ==
        Opcode::Branch))
    builder.CreateBr(mergeBB);

  builder.setInsertPoint(elseBB);

  if (stmt->elseBranch)
    stmt->elseBranch->codegen(*this);
  if (builder.GetInsertBlock()->instructions.size() == 0 ||
      !(builder.GetInsertBlock()->instructions.back()->opcode ==
        Opcode::Branch))
    builder.CreateBr(mergeBB);
  builder.setInsertPoint(mergeBB);

  return nullptr;
}

custom_ir::Value *custom_ir::IRGenerator::generateWhile(WhileStmt *stmt) {
  custom_ir::Function *function = builder.GetInsertBlock()->parent;

  BasicBlockIR *condBB = new BasicBlockIR("while.cond", function);
  BasicBlockIR *bodyBB = new BasicBlockIR("while.body", function);
  BasicBlockIR *afterBB = new BasicBlockIR("while.after", function);

  builder.CreateBr(condBB);
  builder.setInsertPoint(condBB);
  custom_ir::Value *condValue = stmt->condition->codegen(*this);

  if (!condValue)
    throw std::runtime_error("Invalid condition in if");

  if (isNumeric(condValue->dataType)) {
    condValue = builder.CreateBinary(
        Opcode::ICmpNE, condValue,
        builder.CreateConstant(new Type{DATA_TYPE::DATATYPE_INT, nullptr}, "0"),
        "ifcond");
  } else if (condValue->dataType->base == DATA_TYPE::DATATYPE_FLOAT) {
    condValue = builder.CreateBinary(
        Opcode::FCmpNE, condValue,
        builder.CreateConstant(new Type{DATA_TYPE::DATATYPE_FLOAT, nullptr},
                               "0.0"),
        "ifcond");
  } else {
    throw std::runtime_error("Unsupported condition type in if");
  }

  builder.CreateCondBr(condValue, bodyBB, afterBB);

  builder.setInsertPoint(bodyBB);
  breakTargets.push_back(afterBB);
  continueTargets.push_back(condBB);
  //   generateYieldCall();
  stmt->thenBranch->codegen(*this);
  builder.CreateBr(condBB);

  breakTargets.pop_back();
  continueTargets.pop_back();

  builder.setInsertPoint(afterBB);

  return nullptr;
}

custom_ir::Value *custom_ir::IRGenerator::generateBreak(BreakStmt *stmt) {
  builder.CreateBr(breakTargets.back());
  custom_ir::BasicBlockIR *unreachable =
      new BasicBlockIR("after.break", builder.GetInsertBlock()->parent);

  builder.setInsertPoint(unreachable);
  return nullptr;
}

custom_ir::Value *custom_ir::IRGenerator::generateContinue(ContinueStmt *stmt) {
  builder.CreateBr(continueTargets.back());
  custom_ir::BasicBlockIR *unreachable =
      new BasicBlockIR("after.continue", builder.GetInsertBlock()->parent);

  builder.setInsertPoint(unreachable);
  return nullptr;
}
#include <CustomIRBuilder.hpp>
#include <vector>

namespace custom_ir {
int tempCounter = 0;

std::map<int, VariableValue *> variableValues = {};
} // namespace custom_ir

void custom_ir::IRBuilder::setInsertPoint(BasicBlockIR *block) {
  insertPoint = block;
}

void custom_ir::IRBuilder::ClearInsertionPoint() { insertPoint = nullptr; }

custom_ir::BasicBlockIR *custom_ir::IRBuilder::GetInsertBlock() {
  return insertPoint;
}

custom_ir::Instruction *
custom_ir::IRBuilder::insert(custom_ir::Instruction *inst) {
  if (!insertPoint) {
    throw std::runtime_error("No insertion point set");
  }

  insertPoint->instructions.push_back(inst);

  return inst;
}

custom_ir::Value *custom_ir::IRBuilder::CreateCast(custom_ir::Value *val,
                                                   custom_ir::Value *res) {
  std::vector<custom_ir::Value *> operands{val, res};
  custom_ir::Instruction *castInstruction = new Instruction(
      res->dataType->base == DATA_TYPE::DATATYPE_FLOAT ? Opcode::Cast_FLOAT
                                                       : Opcode::Cast_INT,
      operands, res);

  insert(castInstruction);
  return res;
}

custom_ir::Value *custom_ir::IRBuilder::CreateBinary(custom_ir::Opcode op,
                                                     custom_ir::Value *lhs,
                                                     custom_ir::Value *rhs,
                                                     custom_ir::Value *res) {

  auto *inst = new BinaryInstruction(op, lhs, rhs, res);

  insert(inst);
  return res;
}

custom_ir::Value *custom_ir::IRBuilder::CreateStore(custom_ir::Value *var,
                                                    custom_ir::Value *value) {

  std::vector<custom_ir::Value *> operands{value, var};
  custom_ir::Instruction *inst = new Instruction(Opcode::Store, operands, var);

  insert(inst);
  return var;
}

custom_ir::Value *custom_ir::IRBuilder::CreateBitCast(custom_ir::Value *var,
                                                      custom_ir::Value *ptr) {
  std::vector<custom_ir::Value *> operands{var, ptr};
  custom_ir::Instruction *inst =
      new Instruction(Opcode::Bit_Cast, operands, var);

  insert(inst);

  return var;
}

custom_ir::Value *custom_ir::IRBuilder::CreateLoad(Value *val, Value *res) {
  std::vector<custom_ir::Value *> operands{val, res};
  custom_ir::Instruction *inst = new Instruction(Opcode::Load, operands, res);

  insert(inst);

  return res;
}

custom_ir::Value *custom_ir::IRBuilder::CreateConstant(Type *type,
                                                       std::string value) {
  custom_ir::Constant *constVal = new Constant(type, value);

  return constVal;
}

custom_ir::Value *
custom_ir::IRBuilder::CreateConstantSizeof(Type *type, std::string value) {
  custom_ir::ConstantSizeof *constVal = new ConstantSizeof(type, value);

  return constVal;
}

custom_ir::Value *custom_ir::IRBuilder::CreateGlobalVariable(VariableValue *var,
                                                             Value *val) {
  std::vector<custom_ir::Value *> operands{val, var};
  custom_ir::Instruction *inst =
      new Instruction(Opcode::Global_Dec, operands, var);

  return inst;
}

custom_ir::Value *custom_ir::IRBuilder::CreateAlloca(VariableValue *val) {
  std::vector<custom_ir::Value *> operands{val};
  custom_ir::Instruction *inst = new Instruction(Opcode::Alloca, operands, val);

  variableValues[val->variable_id] = val;
  insert(inst);

  return val;
}

custom_ir::Value *custom_ir::IRBuilder::CreateRet(Value *val) {
  std::vector<custom_ir::Value *> operands{val};
  custom_ir::Instruction *inst = new Instruction(Opcode::Return, operands, val);

  insert(inst);

  return val;
}

custom_ir::Value *custom_ir::IRBuilder::CreateCall(Function *func,
                                                   std::string callName,
                                                   std::vector<Value *> args) {
  std::vector<custom_ir::Value *> operands{args};
  operands.push_back(func);
  custom_ir::Instruction *inst = new Instruction(Opcode::Call, operands, func);

  return insert(inst);
}

custom_ir::Value *
custom_ir::IRBuilder::CreateCallExternal(std::string name, Type *type,
                                         std::string callName,
                                         std::vector<Value *> args) {

  std::vector<custom_ir::Value *> operands{args};
  operands.push_back(new Value(type, name));
  custom_ir::Instruction *inst =
      new Instruction(Opcode::CallExternal, operands, new Value(type, name));

  return insert(inst);
}

custom_ir::Value *custom_ir::IRBuilder::CreateCondBr(Value *condValue,
                                                     BasicBlockIR *thenBB,
                                                     BasicBlockIR *elseBB) {
  std::vector<custom_ir::Value *> operands{condValue, thenBB, elseBB};
  custom_ir::Instruction *inst = new Instruction(
      Opcode::CondBranch, operands,
      new Value(new Type{DATA_TYPE::DATATYPE_VOID, nullptr}, "condBranch"));

  BasicBlockIR *currentBB = insertPoint;
  currentBB->successor.push_back(thenBB);
  currentBB->successor.push_back(elseBB);

  thenBB->predecessor.push_back(currentBB);
  elseBB->predecessor.push_back(currentBB);

  return insert(inst);
}

custom_ir::Value *custom_ir::IRBuilder::CreateBr(BasicBlockIR *laterBB) {
  std::vector<custom_ir::Value *> operands{laterBB};
  custom_ir::Instruction *inst = new Instruction(
      Opcode::Branch, operands,
      new Value(new Type{DATA_TYPE::DATATYPE_VOID, nullptr}, "branch"));

  BasicBlockIR *currentBB = insertPoint;
  currentBB->successor.push_back(laterBB);

  laterBB->predecessor.push_back(currentBB);

  return insert(inst);
}

custom_ir::Value *
custom_ir::IRBuilder::CreateGoCall(Function *func, std::string callName,
                                   std::vector<Value *> args) {
  std::vector<custom_ir::Value *> operands{args};
  operands.push_back(func);
  custom_ir::Instruction *inst =
      new Instruction(Opcode::GoCall, operands, func);

  return insert(inst);
}
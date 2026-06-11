#include <CustomIRBuilder.hpp>
#include <vector>

void custom_ir::IRBuilder::setInsertPoint(BasicBlockIR *block) {
  insertPoint = block;
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
                                                   DATA_TYPE newType,
                                                   std::string value) {

  std::vector<custom_ir::Value *> operands{val};
  custom_ir::Instruction *castInstruction =
      new Instruction(newType == DATA_TYPE::DATATYPE_FLOAT ? Opcode::Cast_FLOAT
                                                           : Opcode::Cast_INT,
                      operands, value, new Type{newType, nullptr});

  return insert(castInstruction);
}

custom_ir::Value *custom_ir::IRBuilder::CreateBinary(custom_ir::Opcode op,
                                                     custom_ir::Value *lhs,
                                                     custom_ir::Value *rhs,
                                                     std::string value) {
  auto *inst = new BinaryInstruction(op, lhs, rhs, value, lhs->dataType);

  return insert(inst);
}

custom_ir::Value *custom_ir::IRBuilder::CreateStore(custom_ir::Value *var,
                                                    custom_ir::Value *ptr) {

  std::vector<custom_ir::Value *> operands{var, ptr};
  custom_ir::Instruction *inst =
      new Instruction(Opcode::Store, operands, var->value, var->dataType);

  return insert(inst);
}

custom_ir::Value *custom_ir::IRBuilder::CreateBitCast(custom_ir::Value *var,
                                                      custom_ir::Value *ptr) {
  std::vector<custom_ir::Value *> operands{var, ptr};
  custom_ir::Instruction *inst =
      new Instruction(Opcode::Bit_Cast, operands, var->value, var->dataType);

  return insert(inst);
}

custom_ir::Value *custom_ir::IRBuilder::CreateLoad(Type *type, Value *val,
                                                   std::string name) {
  std::vector<custom_ir::Value *> operands{val};
  custom_ir::Instruction *inst =
      new Instruction(Opcode::Load, operands, name, type);

  return insert(inst);
}

custom_ir::Value *custom_ir::IRBuilder::CreateConstant(Type *type,
                                                       std::string value) {
  custom_ir::Constant *constVal = new Constant(type, value);

  return constVal;
}

custom_ir::Value *custom_ir::IRBuilder::CreateGlobalVariable(std::string name,
                                                             Type *type,
                                                             Value *val) {
  std::vector<custom_ir::Value *> operands{val};
  custom_ir::Instruction *inst =
      new Instruction(Opcode::Global_Dec, operands, name, type);

  return insert(inst);
}

custom_ir::Value *custom_ir::IRBuilder::CreateAlloca(std::string name,
                                                     Type *type) {
  std::vector<custom_ir::Value *> operands{};
  custom_ir::Instruction *inst =
      new Instruction(Opcode::Alloca, operands, name, type);

  return insert(inst);
}
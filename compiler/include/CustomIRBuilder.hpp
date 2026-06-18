#pragma once

#include <CustomIR.hpp>
#include <CustomIRGenerator.hpp>

namespace custom_ir {
class IRBuilder {
  BasicBlockIR *insertPoint;

  Instruction *insert(Instruction *);

public:
  void setInsertPoint(BasicBlockIR *);
  void ClearInsertionPoint();
  BasicBlockIR *GetInsertBlock();

  Value *CreateCast(Value *, Value *);
  Value *CreateBinary(Opcode op, Value *lhs, Value *rhs, Value *res);
  Value *CreateStore(VariableValue *, Value *);
  Value *CreateBitCast(Value *, Value *); // later just use type from the value
  Value *CreateLoad(VariableValue *, Value *);
  Value *CreateConstant(Type *, std::string);
  Value *CreateGlobalVariable(VariableValue *, Value *);
  Value *CreateAlloca(VariableValue *);
  Value *CreateRet(Value *);
  Value *CreateCall(Function *, std::string, std::vector<Value *> args);
  Value *CreateCallExternal(std::string, Type *, std::string,
                            std::vector<Value *> args);
  Value *CreateBr(BasicBlockIR *laterBB);
  Value *CreateCondBr(Value *condValue, BasicBlockIR *thenBB,
                      BasicBlockIR *elseBB);
  Value *CreateGoCall(Function *func, std::string callName,
                      std::vector<Value *> args);
  Value *CreateConstantSizeof(Type *, std::string);
  bool isInsertPointNull() { return insertPoint == nullptr; }
};
} // namespace custom_ir

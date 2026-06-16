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

  Value *CreateCast(Value *, DATA_TYPE, std::string);
  Value *CreateBinary(Opcode op, Value *lhs, Value *rhs, std::string);
  Value *CreateStore(Value *, Value *);
  Value *CreateBitCast(Value *, Value *); // later just use type from the value
  Value *CreateLoad(Type *, Value *, std::string);
  Value *CreateConstant(Type *, std::string);
  Value *CreateGlobalVariable(std::string, Type *, Value *);
  Value *CreateAlloca(std::string, Type *);
  Value *CreateRet(Value *);
  Value *CreateCall(Function *, std::string, std::vector<Value *> args);
  Value *CreateCallExternal(std::string, Type *, std::string,
                            std::vector<Value *> args);
  Value *CreateBr(BasicBlockIR *laterBB);
  Value *CreateCondBr(Value *condValue, BasicBlockIR *thenBB,
                      BasicBlockIR *elseBB);
  Value *CreateGoCall(Function *func, std::string callName,
                      std::vector<Value *> args);
  bool isInsertPointNull() { return insertPoint == nullptr; }
};
} // namespace custom_ir

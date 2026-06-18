#pragma once

#include <Type.hpp>
#include <llvm/IR/Value.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace custom_ir {

class IRGenerator;
class LLVMIRGenerator;

enum class Opcode {
  Add,
  Sub,
  Mul,
  Div,

  FAdd,
  FSub,
  FMul,
  FDiv,

  ICmpEQ,
  ICmpNE,
  ICmpLT,
  ICmpGT,

  FCmpEQ,
  FCmpNE,
  FCmpLT,
  FCmpGT,

  And,
  Or,

  Load,
  Store,

  CondBranch,
  Branch,

  GoCall,
  Call,
  CallExternal,
  Return,

  Cast_INT,
  Cast_FLOAT,
  Bit_Cast,

  Global_Dec,
  Alloca,

  Phi
};

class Function;
class BasicBlockIR;

class Value {
public:
  Type *dataType;
  std::string value;
  Value(Type *dataType, std::string value) : dataType(dataType), value(value) {}
  Value(Value *val) : dataType(val->dataType), value(val->value) {}
  virtual ~Value() = default;
  virtual llvm::Value *llvm_codegen(LLVMIRGenerator *gen);
};

extern int tempCounter;
class TempValue : public Value {

public:
  TempValue(Type *dataType, const std::string &value)
      : Value(dataType, value + std::to_string(tempCounter++)) {}

  llvm::Value *llvm_codegen(LLVMIRGenerator *gen);
};

class VariableValue : public Value {

public:
  int variable_id;
  int version = -1;
  VariableValue(Type *dataType, const std::string &value, int variable_id)
      : Value(dataType, value), variable_id(variable_id) {}

  llvm::Value *llvm_codegen(LLVMIRGenerator *gen);
};

class Constant : public Value {
public:
  Constant(Type *dataType, const std::string &value) : Value(dataType, value) {}
  llvm::Value *llvm_codegen(LLVMIRGenerator *gen);
};

class Instruction : public Value {
public:
  Opcode opcode;

  std::vector<Value *> operands;

  Instruction(Opcode op, std::vector<Value *> operands, Value *res)
      : Value(res), opcode(op), operands(operands) {}
  llvm::Value *llvm_codegen(LLVMIRGenerator *gen);
};

class PhiInstruction : public Instruction {
public:
  int variableId;

  PhiInstruction(int variableId, Value *res)
      : Instruction(Opcode::Phi, {}, res), variableId(variableId) {}
  llvm::Value *llvm_codegen(LLVMIRGenerator *gen);
};

class BinaryInstruction : public Instruction {
public:
  BinaryInstruction(Opcode op, Value *l, Value *r, Value *res)
      : Instruction(op, {l, r, res}, res) {}
  llvm::Value *llvm_codegen(LLVMIRGenerator *gen);
};

class Function : public Value {
public:
  std::string name;
  Type *functionType;

  std::vector<Value *> args;

  std::vector<BasicBlockIR *> blocks;

  Function(std::string name, Type *functionType, std::vector<Value *> args)
      : Value(functionType, name), name(name), args(args),
        functionType(functionType) {}
  llvm::Value *llvm_codegen(LLVMIRGenerator *gen);
};

class BasicBlockIR : public Value {
public:
  std::string name;
  Function *parent;

  std::vector<BasicBlockIR *> predecessor, successor;

  std::vector<Instruction *> instructions;

  BasicBlockIR(std::string name, Function *func)
      : Value(nullptr, name), parent(func), name(name) {
    if (parent != nullptr)
      parent->blocks.push_back(this);
  }

  bool hasTerminator() {
    if (!this->instructions.size() == 0) {
      Opcode op = this->instructions.back()->opcode;
      if (op == Opcode::Branch || op == Opcode::CondBranch ||
          op == Opcode::Return)
        return 1;
    }
    return 0;
  }
  llvm::Value *llvm_codegen(LLVMIRGenerator *gen);
};

class ModuleIR {
public:
  BasicBlockIR *globalDeclarations;
  std::map<std::string, Function *> functions;

  ModuleIR() { globalDeclarations = new BasicBlockIR("global_dec", nullptr); }
  llvm::Value *llvm_codegen(LLVMIRGenerator *gen);
};

class ConstantSizeof : public Value {
public:
  ConstantSizeof(Type *dataType, const std::string &value)
      : Value(dataType, value) {}
  llvm::Value *llvm_codegen(LLVMIRGenerator *gen);
};

extern std::map<int, VariableValue *> variableValues;
}; // namespace custom_ir
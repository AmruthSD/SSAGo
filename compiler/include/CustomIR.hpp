#pragma once

#include <memory>
#include <string>
#include <vector>

namespace custom_ir {
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

  Jump,
  Branch,

  Call,
  Return,

  Cast_INT,
  Cast_FLOAT,
  Bit_Cast,

  Global_Dec,
  Alloca,

  Phi
};

class Value {
public:
  Type *dataType;
  std::string value;
  Value(Type *dataType, std::string value) : dataType(dataType), value(value) {}

  virtual ~Value() = default;
};

class Constant : public Value {
public:
  Constant(Type *dataType, const std::string &value) : Value(dataType, value) {}
};

class Argument : public Value {
public:
  std::string name;
};

class Instruction : public Value {
public:
  Opcode opcode;

  std::vector<Value *> operands;

  Instruction(Opcode op, std::vector<Value *> operands, std::string value,
              Type *dataType)
      : Value(dataType, value), opcode(op), operands(operands) {}
};

class BinaryInstruction : public Instruction {
public:
  BinaryInstruction(Opcode op, Value *l, Value *r, std::string value,
                    Type *dataType)
      : Instruction(op, {l, r}, value, dataType) {}
};

class BasicBlockIR {
public:
  std::string name;

  std::vector<Instruction *> instructions;
};

class FunctionIR {
public:
  std::string name;

  std::vector<std::unique_ptr<Argument>> args;

  std::vector<std::unique_ptr<BasicBlockIR>> blocks;
};

class ModuleIR {
public:
  std::vector<std::unique_ptr<FunctionIR>> functions;
};

}; // namespace custom_ir
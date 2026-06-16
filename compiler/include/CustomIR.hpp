#pragma once

#include <Type.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace custom_ir {
class IRGenerator;

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

  virtual ~Value() = default;
};

class Constant : public Value {
public:
  Constant(Type *dataType, const std::string &value) : Value(dataType, value) {}
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

class Function : public Value {
public:
  std::string name;
  Type *functionType;

  std::vector<Value *> args;

  std::vector<BasicBlockIR *> blocks;

  Function(std::string name, Type *functionType, std::vector<Value *> args)
      : Value(functionType, name), name(name), args(args),
        functionType(functionType) {}
};

class BasicBlockIR : public Value {
public:
  std::string name;
  Function *parent;

  std::vector<BasicBlockIR *> predecessor, successor;

  std::vector<Instruction *> instructions;

  BasicBlockIR(std::string name, Function *func)
      : Value(func->functionType, name), parent(func), name(name) {
    parent->blocks.push_back(this);
  }
};

class ModuleIR {
public:
  std::map<std::string, Function *> functions;
};

}; // namespace custom_ir
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

  Load,
  Store,

  Jump,
  Branch,

  Call,
  Return,

  Phi
};

class Value {
public:
  virtual ~Value() = default;
};

class Constant : public Value {
public:
  int value;
};

class Argument : public Value {
public:
  std::string name;
};

class Instruction : public Value {
public:
  Opcode opcode;

  std::vector<Value *> operands;

  BasicBlockIR *parent;
};

class BinaryInstruction : public Instruction {
public:
  Value *lhs;
  Value *rhs;
};

class BasicBlockIR {
public:
  std::string name;

  std::vector<std::unique_ptr<Instruction>> instructions;
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
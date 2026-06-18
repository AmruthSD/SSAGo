#pragma once

#include <CustomSSAGenerator.hpp>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

using BinaryOpFn = llvm::Value *(llvm::IRBuilder<>::*)(llvm::Value *,
                                                       llvm::Value *,
                                                       const llvm::Twine &);

namespace custom_ir {

struct OpcodeCompare {
  bool operator()(Opcode a, Opcode b) const {
    return static_cast<int>(a) < static_cast<int>(b);
  }
};

class LLVMIRGenerator {
private:
  SSAGenerator &ssaGenerator;

  llvm::LLVMContext context;
  std::unique_ptr<llvm::Module> module;
  llvm::Function *currentFunction;
  llvm::IRBuilder<> builder;
  std::vector<llvm::BasicBlock *> breakTargets;
  std::vector<llvm::BasicBlock *> continueTargets;

  std::vector<std::unordered_map<std::string, llvm::Value *>> namedValues;
  std::unordered_map<llvm::Function *, llvm::Function *> wrapperMap;
  std::map<std::string, llvm::Value *> tempValues;
  std::map<int, llvm::Value *> variableValuesLLVM;
  std::map<BasicBlockIR *, llvm::BasicBlock *> basicBlocks;

public:
  LLVMIRGenerator(SSAGenerator &ssaGenerator);

  llvm::LLVMContext &getContext();
  llvm::Module *getModule();
  llvm::IRBuilder<> &getBuilder();

  void optimizeModule();
  void generateAllExternalFUnctions();
  void writeIRToFile(const std::string &filename);

  llvm::BasicBlock *getOrCreateBasicBlock(BasicBlockIR *block);
  llvm::Value *getTempValue(TempValue *temp);
  llvm::Value *getVariableValue(VariableValue *var);

  llvm::Value *generateAlloca(Instruction *inst);
  llvm::Value *generateLoad(Instruction *inst);
  llvm::Value *generateStore(Instruction *inst);
  llvm::Value *generateCondBranch(Instruction *inst);
  llvm::Value *generateBranch(Instruction *inst);
  llvm::Value *generateReturn(Instruction *);

  llvm::Value *generateModule(ModuleIR *module);
  llvm::Value *generateGlobalVariables(BasicBlockIR *block);

  llvm::Value *getVariablePointer(std::string &name);
  llvm::Value *generateFunction(Function *);
  llvm::Value *generateBlock(BasicBlockIR *);
  llvm::Value *generateInstruction(Instruction *);
  llvm::Value *generateBinaryInstruction(BinaryInstruction *);
  llvm::Value *generatePhiInstruction(PhiInstruction *);
  llvm::Value *generateLiteral(Constant *);
  llvm::Value *generateCast(Instruction *);
  llvm::Value *generateGoFunc(Instruction *);
  llvm::Value *generateFunctionCall(Instruction *);
  llvm::Value *generateExternalCall(Instruction *inst);
  llvm::Value *generateSizeofExpr(ConstantSizeof *);

  llvm::Function *declareExternalFunction(const std::string &name,
                                          llvm::Type *returnType,
                                          std::vector<llvm::Type *> paramTypes,
                                          bool isVarArg);
  llvm::Function *getOrDeclarePrintf();
  llvm::Function *getOrDeclareScanf();
  llvm::Function *getOrDeclareMalloc();
  llvm::Function *getOrDeclareSpawn();
  llvm::Function *getOrDeclareYield();
  llvm::Function *getOrDeclareWaitGroupNew();
  llvm::Function *getOrDeclareWaitGroupAdd();
  llvm::Function *getOrDeclareWaitGroupDone();
  llvm::Function *getOrDeclareWaitGroupWait();
  void generateYieldCall();
  llvm::Function *getOrCreateWrapper(llvm::Function *);
  llvm::Type *getLLVMType(Type *type, llvm::LLVMContext &context);
};

} // namespace custom_ir
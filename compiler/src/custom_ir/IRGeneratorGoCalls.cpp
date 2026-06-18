#include <CustomIRGenerator.hpp>

custom_ir::Value *custom_ir::IRGenerator::generateGoFunc(GoStmt *stmt) {
  std::string funcName = stmt->callee;
  if (external_functions.find(funcName) == external_functions.end())
    funcName = "__user_" + funcName;
  else
    return nullptr;

  Function *func = module->functions[funcName];
  if (!func)
    return nullptr;
  std::vector<custom_ir::Value *> args;
  args.reserve(stmt->arguments.size());

  for (auto &arg : stmt->arguments) {
    custom_ir::Value *argVal = arg->codegen(*this);
    if (!argVal)
      throw std::runtime_error("Failed to generate argument in call to " +
                               funcName);
    args.push_back(argVal);
  }

  Type *ft = func->functionType;
  std::string callName =
      ft->base == DATA_TYPE::DATATYPE_VOID ? "" : funcName + "_call";

  return builder.CreateGoCall(func, callName, args);
}

#include <CustomIRGenerator.hpp>

#include <CommonExternalFunctions.hpp>

custom_ir::Value *custom_ir::IRGenerator::generateFunction(FunctionStmt *func) {

  if (namedValues.size() != 1)
    throw std::runtime_error("Unable to have function as non global");

  std::vector<Value *> paramTypes;
  for (auto &[name, type] : func->arguments)
    paramTypes.push_back(new Value(type, name));

  Type *functionType = func->dataType;

  custom_ir::Function *function =
      new Function("__user_" + func->identifier, functionType, paramTypes);
  module->functions["__user_" + func->identifier] = function;

  BasicBlockIR *entry = new BasicBlockIR("entry", function);

  builder.setInsertPoint(entry);

  namedValues.emplace_back();
  int idx = 0;
  for (auto &arg : function->args) {
    VariableValue *varValue =
        new VariableValue(new Type{DATA_TYPE::DATATYPE_POINTER, arg->dataType},
                          arg->value, variable_id++);

    custom_ir::Value *alloca = builder.CreateAlloca(varValue);

    // builder.CreateStore(arg, alloca);
    namedValues.back()[arg->value] = varValue;
    function->args[idx] = varValue;
    idx++;
  }

  func->body.get()->codegen(*this);

  for (auto &block : function->blocks) {
    if (!block->hasTerminator()) {
      builder.setInsertPoint(block);
      if (function->dataType->base == DATA_TYPE::DATATYPE_VOID) {
        builder.CreateRet(
            new Value(new Type{DATA_TYPE::DATATYPE_VOID, nullptr}, ""));
      } else {
        Type *varType = function->dataType;
        Value *initValue;
        if (varType->pointee != nullptr) {
          initValue = builder.CreateConstant(varType, "null");
        } else if (varType->base == DATA_TYPE::DATATYPE_INT) {
          initValue = builder.CreateConstant(varType, "0");
        } else if (varType->base == DATA_TYPE::DATATYPE_FLOAT) {
          initValue = builder.CreateConstant(varType, "0.0");
        } else {
          throw std::runtime_error("No default return or terminator in block" +
                                   block->name);
        }
        builder.CreateRet(initValue);
      }
    }
  }

  namedValues.pop_back();

  builder.ClearInsertionPoint();
  return nullptr;
}

custom_ir::Value *custom_ir::IRGenerator::generateBlock(BlockStmt *block) {
  if (namedValues.size() == 1)
    throw std::runtime_error("Unable to have block as global");
  namedValues.emplace_back();

  for (auto &stmt : block->body)
    stmt->codegen(*this);

  namedValues.pop_back();
  return nullptr;
}

custom_ir::Value *custom_ir::IRGenerator::generateReturn(ReturnStmt *stmt) {
  custom_ir::Function *function = builder.GetInsertBlock()->parent;
  Type *returnType = function->functionType;

  if (returnType->base == DATA_TYPE::DATATYPE_VOID) {
    if (stmt->expr != nullptr) {
      throw std::runtime_error("Cannot return a value from void function\n");
    }
    return builder.CreateRet(
        new Value(new Type{DATA_TYPE::DATATYPE_VOID, nullptr}, ""));
  }

  if (stmt->expr == nullptr) {
    throw std::runtime_error("Non-void function must return a value\n");
  }

  Value *retValue = stmt->expr->codegen(*this);
  if (!retValue)
    return nullptr;

  if (!areTypesEqual(retValue->dataType, returnType)) {
    if (returnType->base == DATA_TYPE::DATATYPE_FLOAT &&
        retValue->dataType->base == DATA_TYPE::DATATYPE_INT) {
      Value *newRes = new TempValue(returnType, "intToFloatReturn");
      retValue = builder.CreateCast(retValue, newRes);
    } else if (returnType->base == DATA_TYPE::DATATYPE_INT &&
               retValue->dataType->base == DATA_TYPE::DATATYPE_FLOAT) {

      Value *newRes = new TempValue(returnType, "floatToIntReturn");
      builder.CreateCast(retValue, newRes);
    } else {
      throw std::runtime_error("Return type mismatch\n");
    }
  }
  return builder.CreateRet(retValue);
}

custom_ir::Value *custom_ir::IRGenerator::generateFunctionCall(CallExpr *expr) {

  //   generateYieldCall();

  auto *var = dynamic_cast<VariableExpr *>(expr->callee.get());
  if (!var)
    throw std::runtime_error("Invalid function call target in codegen");

  std::string functionName = var->name;

  std::vector<custom_ir::Value *> args;
  args.reserve(expr->arguments.size());

  for (auto &arg : expr->arguments) {
    custom_ir::Value *argVal = arg->codegen(*this);
    if (!argVal)
      throw std::runtime_error("Failed to generate argument in call to " +
                               functionName);
    args.push_back(argVal);
  }

  if (external_functions.find(functionName) == external_functions.end())
    functionName = "__user_" + functionName;
  else {
    return builder.CreateCallExternal(
        functionName, new Type{DATA_TYPE::DATATYPE_VOID, nullptr},
        functionName + "_call", args);
  }

  custom_ir::Function *function = module->functions[functionName];
  if (!function)
    throw std::runtime_error("LLVM function not found: " + functionName);

  Type *ft = function->functionType;
  std::string callName =
      ft->base == DATA_TYPE::DATATYPE_VOID ? "" : functionName + "_call";

  return builder.CreateCall(function, callName, args);
}

custom_ir::Value *custom_ir::IRGenerator::generateSizeofExpr(SizeofExpr *expr) {
  Type *type = expr->dataType;

  return builder.CreateConstantSizeof(expr->dataType, "sizeOfNode");
}
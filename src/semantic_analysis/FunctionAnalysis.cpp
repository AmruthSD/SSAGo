#include <CommonExternalFunctions.hpp>
#include <SemanticAnalyser.hpp>

void SemanticAnalyser::analyseFunctionStmt(FunctionStmt *func) {
  if (symbolTable.find(func->identifier) != symbolTable.end())
    throw std::runtime_error("function identifier is used " + func->identifier);

  std::vector<Type *> argumentTypes;
  for (auto [u, v] : func->arguments) {
    argumentTypes.push_back(v);
  }

  functionSymbolTable[func->identifier] = {func->dataType, argumentTypes};

  current_function_type = func->dataType;

  for (auto param : func->arguments) {
    if (symbolTable.find(param.first) != symbolTable.end())
      throw std::runtime_error("Argument identifier is being used");
    symbolTable[param.first] = {param.second};
  }

  for (auto param : func->arguments) {
    symbolTable.erase(param.first);
  }
}

void SemanticAnalyser::analyseBlock(BlockStmt *stmt) {
  block_number++;
  for (int i = 0; i < stmt->body.size(); i++) {
    stmt->body[i]->analyse(*this);
  }

  for (auto i : symbolTable) {
    if (i.second.block_number == block_number) {
      symbolTable.erase(i.first);
    }
  }
  block_number--;
}

void SemanticAnalyser::analyseReturn(ReturnStmt *stmt) {
  if (block_number == 0)
    throw std::runtime_error("Return cant be in the global scope");

  Type *exprDataType = stmt->expr->analyse(*this);
  if (!areTypesEqual(exprDataType, current_function_type)) {
    stmt->expr = std::make_unique<CastExpr>(std::move(stmt->expr),
                                            current_function_type);
  }
}

Type *SemanticAnalyser::analyseFunctionCall(CallExpr *expr) {
  auto *var = dynamic_cast<VariableExpr *>(expr->callee.get());
  if (!var)
    throw std::runtime_error("Invalid function call target");

  std::string functionName = var->name;
  bool fun_external = false;
  if (external_functions.find(functionName) != external_functions.end())
    fun_external = true;

  if (fun_external) {
    for (int i = 0; i < expr->arguments.size(); i++) {
      Type *expr_type = expr->arguments[i]->analyse(*this);
    }
    return new Type{DATA_TYPE::DATATYPE_VOID, nullptr};
  } else {
    auto it = functionSymbolTable.find(functionName);
    if (it == functionSymbolTable.end())
      throw std::runtime_error("Function not declared: " + functionName);

    FunctionSymbolTableEntry &prop = it->second;
    if (prop.argumentsTypes.size() != expr->arguments.size())
      throw std::runtime_error(
          "Number of arguments are not right in call for " + functionName);

    for (int i = 0; i < prop.argumentsTypes.size(); i++) {
      Type *expr_type = expr->arguments[i]->analyse(*this);
      if (!areTypesEqual(expr_type, prop.argumentsTypes[i]))
        expr->arguments[i] = std::make_unique<CastExpr>(
            std::move(expr->arguments[i]), prop.argumentsTypes[i]);
    }

    return prop.returnType;
  }
}

Type *SemanticAnalyser::analyseSizeofExpr(SizeofExpr *expr) {
  if (expr->dataType == nullptr ||
      expr->dataType->base == DATA_TYPE::DATATYPE_VOID)
    throw std::runtime_error("Sizeof cant have type void be null");

  return new Type{DATA_TYPE::DATATYPE_INT, nullptr};
}
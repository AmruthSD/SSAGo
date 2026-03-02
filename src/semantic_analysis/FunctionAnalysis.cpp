#include <SemanticAnalyser.hpp>

void SemanticAnalyser::analyseFunctionStmt(FunctionStmt *func) {
  if (symbolTable.find(func->identifier) != symbolTable.end())
    throw std::runtime_error("function identifier is used " + func->identifier);

  std::vector<DATA_TYPE> argumentTypes;
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

  DATA_TYPE exprDataType = stmt->expr->analyse(*this);
  if (exprDataType != current_function_type) {
    stmt->expr = std::make_unique<CastExpr>(std::move(stmt->expr),
                                            current_function_type);
  }
}

DATA_TYPE SemanticAnalyser::analyseFunctionCall(CallExpr *expr) {
  auto *var = dynamic_cast<VariableExpr *>(expr->callee.get());
  if (!var)
    throw std::runtime_error("Invalid function call target");

  std::string functionName = var->name;
  auto it = functionSymbolTable.find(functionName);
  if (it == functionSymbolTable.end())
    throw std::runtime_error("Function not declared: " + functionName);

  FunctionSymbolTableEntry &prop = it->second;
  if (prop.argumentsTypes.size() != expr->arguments.size())
    throw std::runtime_error("Number of arguments are not right in call for " +
                             functionName);

  for (int i = 0; i < prop.argumentsTypes.size(); i++) {
    DATA_TYPE expr_type = expr->arguments[i]->analyse(*this);
    if (expr_type != prop.argumentsTypes[i])
      expr->arguments[i] = std::make_unique<CastExpr>(
          std::move(expr->arguments[i]), prop.argumentsTypes[i]);
  }

  return prop.returnType;
}
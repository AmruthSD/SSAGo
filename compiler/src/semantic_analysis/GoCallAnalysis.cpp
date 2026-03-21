#include <SemanticAnalyser.hpp>

void SemanticAnalyser::analyseGoFunc(GoStmt *expr) {
  std::string functionName = expr->callee;
  bool fun_external = false;
  if (external_functions.find(functionName) != external_functions.end())
    fun_external = true;

  if (fun_external) {
    for (int i = 0; i < expr->arguments.size(); i++) {
      Type *expr_type = expr->arguments[i]->analyse(*this);
    }
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
  }
}